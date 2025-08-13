#include "SimpleMapView.h"
#include <algorithm>
#include <cmath>
#include <QRegularExpression>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QWheelEvent>
#include <QDirIterator>
#include <QImageReader>
#include <QProgressBar>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMetaObject>
#include <QDebug>

#define GET_IN_RANGE(v, minv, maxv) (std::min(std::max((v), (minv)), (maxv)))

SimpleMapView::SimpleMapView(QWidget* parent)
	: QWidget(parent),
	m_zoomLevel(17),
	m_minZoomLevel(0),
	m_maxZoomLevel(21),
	m_tileCountPerAxis(1 << m_zoomLevel),
	m_center(39.912341799204775, 32.851170267919244),
	m_tileServer(SimpleMapView::TileServers::INVALID),
	m_tileServerSource(SimpleMapView::TileServerSource::Invalid),
	m_networkManager(this),
	m_tileSize(256),
	m_abortingReplies(false),
	m_tileServerTimer(this),
	m_backupTileServerIndex(0),
	m_lockZoom(false),
	m_lockGeolocation(false),
	m_disableMouseWheelZoom(false),
	m_disableMouseMoveMap(false),
	m_markerIcon(":SimpleMapView/marker.svg")
{
	m_tileServerTimer.setInterval(SimpleMapView::TILE_SERVER_TIMER_INTERVAL_MS);
	(void)m_tileServerTimer.connect(&m_tileServerTimer, &QTimer::timeout, this, &SimpleMapView::checkTileServers);

	this->setTileServer(SimpleMapView::TileServers::OSM);
}

void SimpleMapView::resize(int w, int h)
{
	QWidget::resize(w, h);
	this->updateMap();
}

int SimpleMapView::minZoomLevel() const
{
	return m_minZoomLevel;
}

void SimpleMapView::setMinZoomLevel(int minZoomLevel)
{
	m_minZoomLevel = GET_IN_RANGE(minZoomLevel, 0, m_maxZoomLevel);
	this->setZoomLevel(m_zoomLevel);
}

int SimpleMapView::maxZoomLevel() const
{
	return m_maxZoomLevel;
}

void SimpleMapView::setMaxZoomLevel(int maxZoomLevel)
{
	m_maxZoomLevel = GET_IN_RANGE(maxZoomLevel, m_minZoomLevel, INT_MAX);
	this->setZoomLevel(m_zoomLevel);
}

int SimpleMapView::zoomLevel() const
{
	return m_zoomLevel;
}

void SimpleMapView::setZoomLevel(int zoomLevel)
{
	if (!this->isEnabled() || m_lockZoom) return;

	const int oldZoomLevel = m_zoomLevel;
	m_zoomLevel = GET_IN_RANGE(zoomLevel, m_minZoomLevel, m_maxZoomLevel);

	if (oldZoomLevel != m_zoomLevel)
	{
		m_tileCountPerAxis = 1 << m_zoomLevel;

		this->abortReplies();
		m_tileMap.clear();

		this->updateMap();

		emit this->zoomLevelChanged();
	}
}

qreal SimpleMapView::latitude() const
{
	return m_center.latitude();
}

void SimpleMapView::setLatitude(qreal latitude)
{
	this->setCenter(latitude, this->longitude());
}

qreal SimpleMapView::longitude() const
{
	return m_center.longitude();
}

void SimpleMapView::setLongitude(qreal longitude)
{
	this->setCenter(this->latitude(), longitude);
}

const QGeoCoordinate& SimpleMapView::center() const
{
	return m_center;
}

void SimpleMapView::setCenter(const QGeoCoordinate& center)
{
	this->setCenter(center.latitude(), center.longitude());
}

void SimpleMapView::setCenter(qreal latitude, qreal longitude)
{
	if (!this->isEnabled() || m_lockGeolocation) return;

	bool isChanged = false;

	latitude = GET_IN_RANGE(latitude, -90.0, 90.0);
	longitude = GET_IN_RANGE(longitude, -180.0, 180.0);

	if (latitude != this->latitude())
	{
		m_center.setLatitude(latitude);
		isChanged = true;
	}

	if (longitude != this->longitude())
	{
		m_center.setLongitude(longitude);
		isChanged = true;
	}

	if (isChanged)
	{
		this->updateMap();

		emit this->centerChanged();
	}
}

const QString& SimpleMapView::tileServer() const
{
	return m_tileServer;
}

void SimpleMapView::setTileServer(const QString& tileServer, bool wait)
{
	auto changeTileServer = [this, tileServer](int tileSize)
		{
			const QString oldTileServer = m_tileServer;

			this->abortReplies();
			m_tileMap.clear();

			m_tileSize = tileSize;
			m_tileServer = tileServer;

			auto it = std::remove_if(m_backupTileServers.begin(), m_backupTileServers.end(),
				[tileServer](const QString& backupServer) { return backupServer == tileServer; });
			(void)m_backupTileServers.erase(it, m_backupTileServers.end());
			if (oldTileServer != SimpleMapView::TileServers::INVALID)
			{
				m_backupTileServers.push_back(oldTileServer);
			}
		};

	if (tileServer.startsWith("http"))
	{
		QNetworkRequest request(this->formatTileServerUrlString(tileServer, QPoint(0, 0), 0));
		request.setRawHeader("User-Agent", "Qt/SimpleMapView");
		request.setTransferTimeout(5000);

		QNetworkReply* reply = m_networkManager.get(request);
		auto handleResponse = [this, tileServer, changeTileServer, reply]()
			{
				if (reply->error() == QNetworkReply::NoError)
				{
					QImage tileImage;
					tileImage.loadFromData(reply->readAll());
					changeTileServer(tileImage.width());
					m_tileServerSource = SimpleMapView::TileServerSource::Remote;

					this->updateMap();
					emit this->tileServerChanged();

					reply->deleteLater();
				}
				else
				{
					qDebug() << "[SimpleMapView]" << reply->errorString();
					qDebug() << "[SimpleMapView]" << "failed to set the tile server to" << tileServer;
					reply->deleteLater();
					m_tileServerTimer.start();
				}
			};

		if (wait)
		{
			QEventLoop eventLoop;
			(void)this->connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
			(void)eventLoop.exec();
			handleResponse();
		}
		else
		{
			(void)this->connect(reply, &QNetworkReply::finished, this, handleResponse);
		}
	}
	else
	{
		int tileSize = 0;
		QDirIterator dirIterator(tileServer, { "*.png" }, QDir::Files, QDirIterator::Subdirectories);
		if (dirIterator.hasNext())
		{
			const QString pngPath = dirIterator.next();
			const QImageReader reader(pngPath);
			tileSize = reader.size().width();
		}
		changeTileServer(tileSize);
		m_tileServerSource = (tileServer.startsWith(":")) ? (SimpleMapView::TileServerSource::Resource) : (SimpleMapView::TileServerSource::Local);


		QDir path(m_tileServer);
		path = QDir(path.filePath("{z}"));
		path = QDir(path.filePath("{x}"));
		m_tileServer = path.filePath("{y}.png");

		this->updateMap();
		emit this->tileServerChanged();
	}
}

void SimpleMapView::setTileServer(const std::vector<QString>& tileServers)
{
	for (size_t i = 0; i < tileServers.size(); ++i)
	{
		const QString tileServer = tileServers[i];
		if (tileServer != SimpleMapView::TileServers::INVALID)
		{
			this->setTileServer(tileServer);
			if (m_tileServer == tileServer)
			{
				std::vector<QString> backupServers = tileServers;
				(void)backupServers.erase(backupServers.begin() + i);
				this->addBackupTileServer(backupServers);
				return;
			}
		}
	}
}

SimpleMapView::TileServerSource SimpleMapView::tileServerSource() const
{
	return m_tileServerSource;
}

const std::vector<QString>& SimpleMapView::backupTileServers() const
{
	return m_backupTileServers;
}

void SimpleMapView::addBackupTileServer(const QString& tileServer)
{
	(void)m_backupTileServers.insert(m_backupTileServers.end(), tileServer);
}

void SimpleMapView::addBackupTileServer(const std::vector<QString>& tileServers)
{
	(void)m_backupTileServers.insert(m_backupTileServers.end(), tileServers.begin(), tileServers.end());
}

void SimpleMapView::clearBackupTileServers()
{
	m_backupTileServers.clear();
}

bool SimpleMapView::isZoomLocked() const
{
	return m_lockZoom;
}

void SimpleMapView::lockZoom()
{
	m_lockZoom = true;
}

void SimpleMapView::unlockZoom()
{
	m_lockZoom = false;
}

bool SimpleMapView::isGeolocationLocked() const
{
	return m_lockGeolocation;
}

void SimpleMapView::lockGeolocation()
{
	m_lockGeolocation = true;
}

void SimpleMapView::unlockGeolocation()
{
	m_lockGeolocation = false;
}

bool SimpleMapView::isMouseWheelZoomEnabled() const
{
	return m_disableMouseWheelZoom;
}

void SimpleMapView::enableMouseWheelZoom()
{
	m_disableMouseWheelZoom = false;
}

void SimpleMapView::disableMouseWheelZoom()
{
	m_disableMouseWheelZoom = true;
}

bool SimpleMapView::isMouseMoveMapEnabled() const
{
	return m_disableMouseMoveMap;
}

void SimpleMapView::enableMouseMoveMap()
{
	m_disableMouseMoveMap = false;
}

void SimpleMapView::disableMouseMoveMap()
{
	m_disableMouseMoveMap = true;
}

const QImage& SimpleMapView::markerIcon() const
{
	return m_markerIcon;
}

void SimpleMapView::setMarkerIcon(const QImage& icon)
{
	m_markerIcon = icon;
}

MapImage* SimpleMapView::addMarker(const QGeoCoordinate& position)
{
	MapImage* markerIcon = new MapImage(this);
	markerIcon->setPosition(position);
	markerIcon->setAlignmentFlags(Qt::AlignHCenter | Qt::AlignBottom);
	markerIcon->setImage(m_markerIcon);
	markerIcon->setSize(QSizeF(48.0, 48.0));
	markerIcon->setBackgroundColor(Qt::transparent);

	MapText* markerText = new MapText(markerIcon);
	markerText->setPosition(position);
	markerText->setAlignmentFlags(Qt::AlignHCenter | Qt::AlignBottom);
	markerText->setBackgroundColor(QColor::fromRgba(0xAF000000));
	markerText->setBorderRadius(8);
	markerText->setTextColor(Qt::white);

	auto adjustMarkerTextPosition = [this, markerText, markerIcon]()
		{
			QPointF screenPosition = markerIcon->position().screenPoint(this);
			screenPosition.ry() -= markerIcon->size().screenSize(this, screenPosition).height();
			screenPosition.ry() -= markerText->textPadding().bottom() + 5;
			markerText->setPosition(this->screenPositionToGeoCoordinate(screenPosition));
		};

	(void)markerIcon->connect(markerIcon, &MapImage::positionChanged, adjustMarkerTextPosition);
	(void)markerIcon->connect(this, &SimpleMapView::zoomLevelChanged, adjustMarkerTextPosition);
	adjustMarkerTextPosition();

	return markerIcon;
}

MapImage* SimpleMapView::addMarker(qreal latitude, qreal longitude)
{
	return this->addMarker(QGeoCoordinate(latitude, longitude));
}

void SimpleMapView::downloadTiles(const QString& path, const QGeoCoordinate& p1, const QGeoCoordinate& p2, int z1, int z2)
{
	// This is on purpose, My heart demons told me to write this code.

	if (m_tileServer == SimpleMapView::TileServers::INVALID)
	{
		qDebug() << "[SimpleMapView]" << "Tile server is not set.";
		return;
	}

	int z = std::min(z1, z2);
	const int z_end = std::max(z1, z2);
	const int originalZ = m_zoomLevel;

	const QDir dir(path);
	if (!dir.exists())
	{
		if (!dir.mkpath("."))
		{
			qDebug() << "[SimpleMapView]" << "Failed to create the directory to save the tiles";
			return;
		}
	}

	// calculate the number of tiles to download
	size_t nTilesToDownload = 0;
	do
	{
		this->setZoomLevel(z);

		const QPoint requiredTileCount = this->calcRequiredTileCount();
		const QPointF tp1 = this->geoCoordinateToTilePosition(p1);
		const QPointF tp2 = this->geoCoordinateToTilePosition(p2);

		const int x_start = std::min(tp1.x(), tp2.x()) + ((-requiredTileCount.x() / 2.0f) - 1);
		const size_t x_end = std::max(tp1.x(), tp2.x()) + ((requiredTileCount.x() / 2.0f) + 1);
		const int y_start = std::min(tp1.y(), tp2.y()) + ((-requiredTileCount.y() / 2.0f) - 1);
		const size_t y_end = std::max(tp1.y(), tp2.y()) + ((requiredTileCount.y() / 2.0f) + 1);
		nTilesToDownload += (x_end - x_start + 1) * (y_end - y_start + 1);

		z++;
	} while (z <= z_end);
	z = std::min(z1, z2);

	// shared so they won't be deleted when the function returns.
	std::shared_ptr<QFile> qrcFile(new QFile(dir.filePath("Tiles.qrc")));
	std::shared_ptr<QTextStream> qrcTextStream;
	if (!qrcFile->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qrcFile = nullptr;
		qrcTextStream = nullptr;
	}
	else
	{
		qrcTextStream = std::make_shared<QTextStream>(qrcFile.get());
	}

	if (qrcFile != nullptr)
	{
		(*qrcTextStream) << "<RCC>\n";
		(*qrcTextStream) << "\t<qresource prefix=\"/SimpleMapView/Tiles\">\n";
	}

	std::shared_ptr<size_t> currentRequestCount(new size_t(0));
	std::shared_ptr<size_t> downloadedTileCount(new size_t(0));
	std::shared_ptr<QProgressBar> progressBar(new QProgressBar(nullptr));
	progressBar->setWindowTitle("Downloading Tiles");
	progressBar->resize(300, 200);
	progressBar->setRange(0, nTilesToDownload);
	progressBar->setValue(0);
	progressBar->show();

	std::shared_ptr<std::function<void(size_t, size_t, int)>> downloadNextTile = std::make_shared<std::function<void(size_t, size_t, int)>>();

	std::shared_ptr<int> x_start(new int(0));
	std::shared_ptr<size_t> x_end(new size_t(0));
	std::shared_ptr<int> y_start(new int(0));
	std::shared_ptr<size_t> y_end(new size_t(0));
	std::shared_ptr<size_t> x_next(new size_t(0));
	std::shared_ptr<size_t> y_next(new size_t(0));
	std::shared_ptr<size_t> z_next(new size_t(0));

	auto increaseTilePosition = [this, x_next, y_next, z_next, p1, p2, x_start, y_start, x_end, y_end, z_end]()
		{
			(*y_next) += 1;
			if ((*y_next) > (*y_end))
			{
				(*y_next) = (*y_start);
				(*x_next) += 1;
				if ((*x_next) > (*x_end))
				{
					(*x_next) = (*x_start);
					(*z_next) += 1;
					if ((*z_next) <= z_end)
					{
						this->setZoomLevel(*z_next);
						const QPoint requiredTileCount = this->calcRequiredTileCount();
						const QPointF tp1 = this->geoCoordinateToTilePosition(p1);
						const QPointF tp2 = this->geoCoordinateToTilePosition(p2);

						(*x_start) = std::min(tp1.x(), tp2.x()) + ((-requiredTileCount.x() / 2.0f) - 1);
						(*x_end) = std::max(tp1.x(), tp2.x()) + ((requiredTileCount.x() / 2.0f) + 1);
						(*y_start) = std::min(tp1.y(), tp2.y()) + ((-requiredTileCount.y() / 2.0f) - 1);
						(*y_end) = std::max(tp1.y(), tp2.y()) + ((requiredTileCount.y() / 2.0f) + 1);
						(*x_next) = (*x_start);
						(*y_next) = (*y_start);
					}
				}
			}
		};

	(*downloadNextTile) =
		[this, downloadNextTile, increaseTilePosition, qrcFile, qrcTextStream, x_next, y_next, z_next, p1, p2, x_start, y_start, x_end, y_end, z_end, currentRequestCount, originalZ, dir, progressBar, downloadedTileCount](size_t x, size_t y, int z)
		{
			++(*currentRequestCount);

			const QPoint tilePosition(x, y);
			const QString tileKey = this->getTileKey(tilePosition);
			QDir fullDir(QDir(dir.filePath(QString("%1").arg(z))).filePath(QString("%1").arg(x)));
			const QString tilePath = fullDir.filePath(QString("%1.png").arg(y));

			if (!fullDir.exists() && !fullDir.mkpath("."))
			{
				qDebug() << "[SimpleMapView]" << QString("Failed to save the tile z:%1 x:%2 y:%3").arg(z).arg(x).arg(y);
			}

			if (this->validateTilePosition(tilePosition) &&
				!QFile::exists(tilePath))
			{
				QNetworkRequest request(this->formatTileServerUrlString(m_tileServer, tilePosition, m_zoomLevel));
				request.setRawHeader("User-Agent", "Qt/SimpleMapView");
				request.setTransferTimeout(5000);

				QNetworkReply* reply = m_networkManager.get(request);

				(void)this->connect(reply, &QNetworkReply::finished, this,
					[this, downloadNextTile, increaseTilePosition, qrcFile, qrcTextStream, x_next, y_next, z_next, p1, p2, x_start, y_start, x_end, y_end, z_end, currentRequestCount, originalZ, dir, progressBar, downloadedTileCount, reply, x, y, z, tilePath]()
					{
						if (reply->error() == QNetworkReply::NoError)
						{
							std::unique_ptr<QImage> tileImage = std::make_unique<QImage>();
							tileImage->loadFromData(reply->readAll());
							if (!tileImage->save(tilePath))
							{
								qDebug() << "[SimpleMapView]" << QString("Failed to save the tile z:%1 x:%2 y:%3").arg(z).arg(x).arg(y);
							}
							else
							{
								(*qrcTextStream) << QString("\t\t<file alias=\"/%1/%2/%3.png\">%1/%2/%3.png</file>\n").arg(z).arg(x).arg(y);
							}
						}
						else
						{
							qDebug() << "[SimpleMapView]" << "DOWNLOAD ERROR" << reply->errorString();
						}

						(*downloadedTileCount)++;
						progressBar->setValue(*downloadedTileCount);

						(*currentRequestCount)--;
						(*downloadNextTile)(*x_next, *y_next, *z_next);
						increaseTilePosition();
						if (*(z_next) > z_end)
						{
							reply->deleteLater();
							this->setZoomLevel(originalZ);

							(*qrcTextStream) << "\t</qresource>\n";
							(*qrcTextStream) << "</RCC>\n";
							qrcFile->close();

							return;
						}

						reply->deleteLater();
					}
				);
			}
			else
			{
				// execute after the current event loop iteration finishes
				(void)QMetaObject::invokeMethod(this,
					[this, downloadNextTile, increaseTilePosition, qrcFile, qrcTextStream, x_next, y_next, z_next, z_end, originalZ, currentRequestCount, dir, progressBar, downloadedTileCount]()
					{
						(*downloadedTileCount)++;
						progressBar->setValue(*downloadedTileCount);

						(*currentRequestCount)--;
						(*downloadNextTile)(*x_next, *y_next, *z_next);
						increaseTilePosition();
						if (*(z_next) > z_end)
						{
							this->setZoomLevel(originalZ);

							(*qrcTextStream) << "\t</qresource>\n";
							(*qrcTextStream) << "</RCC>\n";
							qrcFile->close();

							return;
						}
					},
					Qt::QueuedConnection
				);
			}
		};


	do
	{
		this->setZoomLevel(z);
		const QPoint requiredTileCount = this->calcRequiredTileCount();
		const QPointF tp1 = this->geoCoordinateToTilePosition(p1);
		const QPointF tp2 = this->geoCoordinateToTilePosition(p2);

		(*x_start) = std::min(tp1.x(), tp2.x()) + ((-requiredTileCount.x() / 2.0f) - 1);
		(*x_end) = std::max(tp1.x(), tp2.x()) + ((requiredTileCount.x() / 2.0f) + 1);
		(*y_start) = std::min(tp1.y(), tp2.y()) + ((-requiredTileCount.y() / 2.0f) - 1);
		(*y_end) = std::max(tp1.y(), tp2.y()) + ((requiredTileCount.y() / 2.0f) + 1);
		(*x_next) = (*x_start);
		(*y_next) = (*y_start);
		(*z_next) = z;

		for (size_t x = std::max(*x_start, 0); x <= (*x_end) && (*currentRequestCount) < SimpleMapView::DOWNLOAD_MAX_CONCURRENT_REQUEST_COUNT; ++x)
		{
			for (size_t y = std::max(*y_start, 0); y <= (*y_end) && (*currentRequestCount) < SimpleMapView::DOWNLOAD_MAX_CONCURRENT_REQUEST_COUNT; ++y)
			{
				(*downloadNextTile)(x, y, z);
				increaseTilePosition();
			}
		}

		z++;
	} while (z <= z_end && (*currentRequestCount) < SimpleMapView::DOWNLOAD_MAX_CONCURRENT_REQUEST_COUNT);
}

QPointF SimpleMapView::geoCoordinateToTilePosition(qreal latitude, qreal longitude) const
{
	const qreal x = ((longitude + 180.0) / (360.0)) * m_tileCountPerAxis;
	const qreal y = ((1.0 - (log(tan(M_PI_4 + (qDegreesToRadians(latitude) / 2.0))) / M_PI)) / 2.0) * m_tileCountPerAxis;

	return QPointF(x, y);
}

QPointF SimpleMapView::geoCoordinateToTilePosition(const QGeoCoordinate& geoCoordinate) const
{
	return this->geoCoordinateToTilePosition(geoCoordinate.latitude(), geoCoordinate.longitude());
}

QPointF SimpleMapView::geoCoordinateToScreenPosition(qreal latitude, qreal longitude) const
{
	return this->tilePositionToScreenPosition(this->geoCoordinateToTilePosition(latitude, longitude));
}

QPointF SimpleMapView::geoCoordinateToScreenPosition(const QGeoCoordinate& geoCoordinate) const
{
	return this->geoCoordinateToScreenPosition(geoCoordinate.latitude(), geoCoordinate.longitude());
}

QGeoCoordinate SimpleMapView::tilePositionToGeoCoordinate(const QPointF& tilePosition) const
{
	const qreal longitude = tilePosition.x() * (360.0 / m_tileCountPerAxis) - 180.0;
	const qreal latitude = qRadiansToDegrees(2.0 * (atan(exp(-M_PI * (tilePosition.y() * (2.0 / m_tileCountPerAxis) - 1))) - M_PI_4));

	return QGeoCoordinate(latitude, longitude);
}

QPointF SimpleMapView::tilePositionToScreenPosition(const QPointF& tilePosition) const
{
	const QPointF relativeTilePosition = tilePosition - this->geoCoordinateToTilePosition(m_center);

	const qreal x = (this->width() / 2.0) + (relativeTilePosition.x() * m_tileSize);
	const qreal y = (this->height() / 2.0) + (relativeTilePosition.y() * m_tileSize);

	return QPointF(x, y);
}

QPointF SimpleMapView::screenPositionToTilePosition(const QPointF& screenPosition) const
{
	const QPointF centerTilePosition = this->geoCoordinateToTilePosition(m_center);

	const qreal x = ((screenPosition.x() - (this->width() / 2.0)) / m_tileSize) + centerTilePosition.x();
	const qreal y = ((screenPosition.y() - (this->height() / 2.0)) / m_tileSize) + centerTilePosition.y();

	return QPointF(x, y);
}

QGeoCoordinate SimpleMapView::screenPositionToGeoCoordinate(const QPointF& screenPosition) const
{
	return this->tilePositionToGeoCoordinate(this->screenPositionToTilePosition(screenPosition));
}

QPoint SimpleMapView::calcRequiredTileCount() const
{
	const int x = ceil(((qreal)this->width()) / m_tileSize);
	const int y = ceil(((qreal)this->height()) / m_tileSize);

	return QPoint(x, y);
}

bool SimpleMapView::validateTilePosition(const QPoint& tilePosition) const
{
	return QRect(0, 0, m_tileCountPerAxis, m_tileCountPerAxis).contains(tilePosition);
}

QString SimpleMapView::getTileKey(const QPoint& tilePosition) const
{
	return QString("%1 %2 %3").arg(tilePosition.x()).arg(tilePosition.y()).arg(m_zoomLevel);
}

QPoint SimpleMapView::getTilePosition(const QString& tileKey, int* outZoomLevel) const
{
	const QStringList splitKey = tileKey.split(" ");
	if (splitKey.size() < 3)
	{
		qDebug() << "[SimpleMapView]" << "invalid tile key";
		if (outZoomLevel != nullptr)
		{
			(*outZoomLevel) = 0;
		}
		return QPoint();
	}

	if (outZoomLevel != nullptr)
	{
		(*outZoomLevel) = splitKey[2].toInt();
	}

	return QPoint(splitKey[0].toInt(), splitKey[1].toInt());
}

QString SimpleMapView::formatTileServerUrlString(QString tileServerUrl, const QPoint& tilePosition, int zoomLevel) const
{
	tileServerUrl.replace("{x}", QString::number(tilePosition.x()));
	tileServerUrl.replace("{y}", QString::number(tilePosition.y()));
	tileServerUrl.replace("{z}", QString::number(zoomLevel));
	return tileServerUrl;
}

void SimpleMapView::updateMap()
{
	if (m_tileServer == SimpleMapView::TileServers::INVALID || m_tileServerSource == SimpleMapView::TileServerSource::Invalid) return;

	const QPoint requiredTileCount = this->calcRequiredTileCount();
	const QPointF centerTilePosition = this->geoCoordinateToTilePosition(m_center);

	const int x_start = (-requiredTileCount.x() / 2) - 1;
	const int x_end = (requiredTileCount.x() / 2) + 1;
	const int y_start = (-requiredTileCount.y() / 2) - 1;
	const int y_end = (requiredTileCount.y() / 2) + 1;

	bool noNewTiles = true;
	for (int x = x_start; x <= x_end; ++x)
	{
		for (int y = y_start; y <= y_end; ++y)
		{
			const QPoint tilePosition(x + centerTilePosition.x(), y + centerTilePosition.y());
			const QString tileKey = this->getTileKey(tilePosition);

			if (this->validateTilePosition(tilePosition) &&
				m_replyMap.find(tileKey) == m_replyMap.end() &&
				m_tileMap.find(tileKey) == m_tileMap.end())
			{
				noNewTiles = false;
				this->fetchTile(tilePosition);
			}
		}
	}
	if (noNewTiles || m_tileServerSource != SimpleMapView::TileServerSource::Remote) this->repaint();
}

void SimpleMapView::fetchTile(const QPoint& tilePosition)
{
	if (m_tileServer == SimpleMapView::TileServers::INVALID) return;
	switch (m_tileServerSource)
	{
	case SimpleMapView::TileServerSource::Remote:
		this->fetchTileFromRemote(tilePosition);
		break;
	case SimpleMapView::TileServerSource::Local:
		this->fetchTileFromLocal(tilePosition);
		break;
	case SimpleMapView::TileServerSource::Resource:
		this->fetchTileFromResource(tilePosition);
		break;
	default:
		break;
	}
}

void SimpleMapView::fetchTileFromRemote(const QPoint& tilePosition)
{
	QNetworkRequest request(this->formatTileServerUrlString(m_tileServer, tilePosition, m_zoomLevel));
	request.setRawHeader("User-Agent", "Qt/SimpleMapView");
	request.setTransferTimeout(5000);

	const QString tileKey = this->getTileKey(tilePosition);
	QNetworkReply* reply = m_networkManager.get(request);
	m_replyMap[tileKey] = reply;

	(void)this->connect(reply, &QNetworkReply::finished, this,
		[this, reply, tileKey]()
		{
			if (reply->error() == QNetworkReply::NoError)
			{
				std::unique_ptr<QImage> tileImage = std::make_unique<QImage>();
				tileImage->loadFromData(reply->readAll());

				m_tileSize = tileImage->width();
				m_tileMap[tileKey] = std::move(tileImage);
			}
			else
			{
				qDebug() << "[SimpleMapView]" << reply->errorString();
				if (!m_abortingReplies)
				{
					m_backupTileServerIndex = 0;
					m_tileServerTimer.start();
				}
			}

			reply->deleteLater();
			if (!m_abortingReplies) // m_replyMap will be cleared after abort
			{
				(void)m_replyMap.erase(tileKey);
			}

			if (m_replyMap.size() == 0)
			{
				this->repaint();
			}
		}
	);
}

void SimpleMapView::fetchTileFromLocal(const QPoint& tilePosition)
{
	const QString tilePath = this->formatTileServerUrlString(m_tileServer, tilePosition, m_zoomLevel);
	if (QFile::exists(tilePath))
	{
		const QString tileKey = this->getTileKey(tilePosition);

		std::unique_ptr<QImage> tileImage = std::make_unique<QImage>();
		tileImage->load(tilePath);

		m_tileSize = tileImage->width();
		m_tileMap[tileKey] = std::move(tileImage);
	}
}

void SimpleMapView::fetchTileFromResource(const QPoint& tilePosition)
{
	this->fetchTileFromLocal(tilePosition);
}

void SimpleMapView::abortReplies()
{
	m_abortingReplies = true;

	for (auto& p : m_replyMap)
	{
		if (!p.second->isFinished())
		{
			p.second->abort();
		}
	}

	m_abortingReplies = false;
	m_replyMap.clear();
}

std::vector<QString> SimpleMapView::visibleTiles() const
{
	std::vector<QString> tileKeys;
	tileKeys.reserve(m_tileMap.size());

	const QRectF renderRect(0, 0, this->width(), this->height());

	for (const auto& tile : m_tileMap)
	{
		const QPointF screenPosition = this->tilePositionToScreenPosition(this->getTilePosition(tile.first));
		const QRectF tileRect(screenPosition, QSizeF(m_tileSize, m_tileSize));

		if (tileRect.intersects(renderRect))
		{
			tileKeys.push_back(tile.first);
		}
	}

	return tileKeys;
}

void SimpleMapView::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	painter.setRenderHint(QPainter::LosslessImageRendering);

	const QPainterPath painterPath = this->calcPaintClipRegion();
	painter.setClipPath(painterPath);

	// fill background
	painter.fillRect(event->region().boundingRect(), painter.background());

	// draw tiles
	for (const auto& tileKey : this->visibleTiles())
	{
		const QPointF screenPosition = this->tilePositionToScreenPosition(this->getTilePosition(tileKey));
		painter.drawImage(screenPosition.x(), screenPosition.y(), *m_tileMap[tileKey]);
	}

	std::function<void(QObject*)> drawItems = [&painter, &drawItems](QObject* parent)
		{
			for (QObject* child : parent->children())
			{
				if (child->inherits("MapItem"))
				{
					((MapItem*)child)->paint(painter);
				}
				drawItems(child);
			}
		};
	drawItems(this);

	// draw border
	painter.setPen(this->extractBorderPenFromStyleSheet());
	painter.drawPath(painterPath);
}

void SimpleMapView::wheelEvent(QWheelEvent* event)
{
	if (!this->isEnabled() || m_disableMouseWheelZoom || m_lockZoom) return;

	const int angleDelta = event->angleDelta().y();
	if (angleDelta > 0)
		this->setZoomLevel(this->zoomLevel() + 1);
	else if (angleDelta < 0)
		this->setZoomLevel(this->zoomLevel() - 1);
}

void SimpleMapView::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		m_lastMousePosition = event->pos();
	}
}

void SimpleMapView::mouseMoveEvent(QMouseEvent* event)
{
	if (!this->isEnabled() || m_disableMouseMoveMap || m_lockGeolocation) return;

	if (event->buttons() & Qt::LeftButton)
	{
		const QPoint currentMousePosition = event->pos();
		const QPoint deltaMousePosition = currentMousePosition - m_lastMousePosition;

		// 1.0 / (dx/dlongitude)
		const qreal deltaLongitude = 360.0 / m_tileCountPerAxis;

		// 1.0 / (dy/dlatitude)
		const qreal deltaLatitude = -(360.0 / m_tileCountPerAxis) * cos(qDegreesToRadians(this->latitude()));

		this->setCenter(
			(this->latitude() - (deltaMousePosition.y() * (deltaLatitude / m_tileSize))),
			(this->longitude() - (deltaMousePosition.x() * (deltaLongitude / m_tileSize)))
		);

		m_lastMousePosition = currentMousePosition;
	}
}

void SimpleMapView::checkTileServers()
{
	constexpr bool wait = false;

	m_tileServerTimer.stop();

	if (m_backupTileServerIndex > m_backupTileServers.size())
		m_backupTileServerIndex = 0;

	if (m_backupTileServerIndex == 0)
	{
		if (m_tileServer != SimpleMapView::TileServers::INVALID)
		{
			this->setTileServer(m_tileServer, wait);
		}
		else
		{
			m_backupTileServerIndex++;
		}
	}

	if (m_backupTileServers.size() > 0)
	{
		if (m_backupTileServerIndex > 0)
		{
			this->setTileServer(m_backupTileServers[m_backupTileServerIndex - 1], wait);
		}
	}

	m_backupTileServerIndex++;
}

QPainterPath SimpleMapView::calcPaintClipRegion() const
{
	const std::array<int, 4> radii = this->extractBorderRadiiFromStyleSheet();
	const int topLeftRadius = radii[0];
	const int topRightRadius = radii[1];
	const int bottomRightRadius = radii[2];
	const int bottomLeftRadius = radii[3];

	QPainterPath painterPath;

	painterPath.moveTo(topLeftRadius, 0);
	painterPath.lineTo(width() - topRightRadius, 0);

	painterPath.quadTo(width(), 0, width(), topRightRadius);
	painterPath.lineTo(width(), height() - bottomRightRadius);

	painterPath.quadTo(width(), height(), width() - bottomRightRadius, height());
	painterPath.lineTo(bottomLeftRadius, height());

	painterPath.quadTo(0, height(), 0, height() - bottomLeftRadius);
	painterPath.lineTo(0, topLeftRadius);

	painterPath.quadTo(0, 0, topLeftRadius, 0);

	painterPath.closeSubpath();

	return painterPath;
}

std::array<int, 4> SimpleMapView::extractBorderRadiiFromStyleSheet() const
{
	std::array<int, 4> radii = { 0, 0, 0, 0 };
	QString styleSheet = this->styleSheet();
	QRegularExpression radiiRegex("(border-radius|border-top-left-radius|border-top-right-radius|border-bottom-right-radius|border-bottom-left-radius)\\s*:\\s*([^;]+);");

	// search to extract border radius values
	QRegularExpressionMatchIterator matchIterator = radiiRegex.globalMatch(styleSheet);

	// if radius of a corner is explicitly set (e.g. border-top-left-radius)
	// use it, if not use the "border-radius" parameter
	std::array<bool, 4> isRadiiSet = { false, false, false, false };

	while (matchIterator.hasNext())
	{
		QRegularExpressionMatch match = matchIterator.next();
		QString property = match.captured(1);
		int value = match.captured(2).remove("px").toInt();

		if (property == "border-radius")
		{
			if (!isRadiiSet[0]) radii[0] = value;
			if (!isRadiiSet[1]) radii[1] = value;
			if (!isRadiiSet[2]) radii[2] = value;
			if (!isRadiiSet[3]) radii[3] = value;
		}
		else if (property == "border-top-left-radius")
		{
			radii[0] = value;
			isRadiiSet[0] = true;
		}
		else if (property == "border-top-right-radius")
		{
			radii[1] = value;
			isRadiiSet[1] = true;
		}
		else if (property == "border-bottom-right-radius")
		{
			radii[2] = value;
			isRadiiSet[2] = true;
		}
		else if (property == "border-bottom-left-radius")
		{
			radii[3] = value;
			isRadiiSet[3] = true;
		}
	}

	return radii;
}

QPen SimpleMapView::extractBorderPenFromStyleSheet() const
{
	QPen pen(Qt::transparent);
	QString styleSheet = this->styleSheet();
	QRegularExpression borderColorRegex("(border|border-color)\\s*:\\s*([^;]+);");
	QRegularExpression borderWidthRegex("(border|border-width)\\s*:\\s*([^;]+);");
	QRegularExpression borderStyleRegex("(border|border-style)\\s*:\\s*([^;]+);");

	// get color
	QRegularExpressionMatchIterator matchIterator = borderColorRegex.globalMatch(styleSheet);
	while (matchIterator.hasNext())
	{
		QRegularExpressionMatch match = matchIterator.next();
		QString property = match.captured(1);
		QString value = match.captured(2);

		if (property == "border")
		{
			for (const QString& s : value.split(" "))
			{
				const QColor c = QColor::fromString(s);
				if (c.isValid())
				{
					pen.setColor(c);
					break;
				}
			}
		}
		else if (property == "border-color")
		{
			const QColor c = QColor::fromString(value);
			if (c.isValid())
			{
				pen.setColor(c);
				break;
			}
		}
	}

	// get width
	matchIterator = borderWidthRegex.globalMatch(styleSheet);
	while (matchIterator.hasNext())
	{
		QRegularExpressionMatch match = matchIterator.next();
		QString property = match.captured(1);
		QString value = match.captured(2).remove("px");

		if (property == "border")
		{
			for (const QString& s : value.split(" "))
			{
				bool ok = false;
				const int bw = s.toInt(&ok);
				if (ok)
				{
					pen.setWidth(bw);
					break;
				}
			}
		}
		else if (property == "border-width")
		{
			bool ok = false;
			const int bw = value.toInt(&ok);
			if (ok)
			{
				pen.setWidth(bw);
				break;
			}
		}
	}

	// get style
	matchIterator = borderStyleRegex.globalMatch(styleSheet);
	while (matchIterator.hasNext())
	{
		QRegularExpressionMatch match = matchIterator.next();
		QString property = match.captured(1);
		QString value = match.captured(2);

		if (property == "border")
		{
			for (const QString& s : value.split(" "))
			{
				if (s == "solid" || s == "none")
				{
					pen.setStyle(Qt::PenStyle::SolidLine);
					break;
				}
				else if (s == "dash")
				{
					pen.setStyle(Qt::PenStyle::DashLine);
					break;
				}
				else if (s == "dot")
				{
					pen.setStyle(Qt::PenStyle::DotLine);
					break;
				}
			}
		}
		else if (property == "border-style")
		{
			if (value == "solid" || value == "none")
			{
				pen.setStyle(Qt::PenStyle::SolidLine);
				break;
			}
			else if (value == "dash")
			{
				pen.setStyle(Qt::PenStyle::DashLine);
				break;
			}
			else if (value == "dot")
			{
				pen.setStyle(Qt::PenStyle::DotLine);
				break;
			}
		}
	}

	return pen;
}