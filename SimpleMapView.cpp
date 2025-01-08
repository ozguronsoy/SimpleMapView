#include "SimpleMapView.h"
#include <algorithm>
#include <cmath>
#include <QEventLoop>
#include <QPainter>
#include <QWheelEvent>
#include <QDebug>

SimpleMapView::SimpleMapView(QWidget* parent)
	: QWidget(parent),
	m_minZoomLevel(0),
	m_maxZoomLevel(21),
	m_zoomLevel(17),
	m_tileCountPerAxis(1 << m_zoomLevel),
	m_tileServer(SimpleMapView::INVALID_TILE_SERVER),
	m_center(39.912341799204775, 32.851170267919244),
	m_networkManager(this),
	m_tileSize(256),
	m_abortingReplies(false),
	m_lockZoom(false),
	m_lockGeolocation(false),
	m_disableMouseWheelZoom(false),
	m_disableMouseMoveMap(false)
{
	this->setTileServer("https://a.tile.openstreetmap.org/{z}/{x}/{y}.png");
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
	assert((void("zoomLevel cannot be negative"), minZoomLevel >= 0));
	assert((void("minZoomLevel cannot be greater than m_maxZoomLevel"), minZoomLevel <= m_maxZoomLevel));

	m_minZoomLevel = minZoomLevel;
	this->setZoomLevel(m_zoomLevel);
}

int SimpleMapView::maxZoomLevel() const
{
	return m_maxZoomLevel;
}

void SimpleMapView::setMaxZoomLevel(int maxZoomLevel)
{
	assert((void("zoomLevel cannot be negative"), maxZoomLevel >= 0));
	assert((void("maxZoomLevel cannot be less than m_minZoomLevel"), maxZoomLevel >= m_minZoomLevel));

	m_maxZoomLevel = maxZoomLevel;
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
	m_zoomLevel = std::min(std::max(zoomLevel, m_minZoomLevel), m_maxZoomLevel);

	if (oldZoomLevel != m_zoomLevel)
	{
		m_tileCountPerAxis = 1 << m_zoomLevel;

		this->abortReplies();
		m_tileMap.clear();

		this->updateMap();
	}
}

double SimpleMapView::latitude() const
{
	return m_center.latitude();
}

void SimpleMapView::setLatitude(double latitude)
{
	this->setCenter(latitude, this->longitude());
}

double SimpleMapView::longitude() const
{
	return m_center.longitude();
}

void SimpleMapView::setLongitude(double longitude)
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

void SimpleMapView::setCenter(double latitude, double longitude)
{
	if (!this->isEnabled() || m_lockGeolocation) return;

	assert((void("latitude must be between -90 to 90 inclusive"), (latitude >= -90) && (latitude <= 90)));
	assert((void("longitude must be between -180 to 180 inclusive"), (longitude >= -180) && (longitude <= 180)));

	bool isChanged = false;

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
	}
}

const QString& SimpleMapView::tileServer() const
{
	return m_tileServer;
}

void SimpleMapView::setTileServer(const QString& tileServer)
{
	if (tileServer == m_tileServer) return;

	this->abortReplies();
	m_tileMap.clear();
	m_tileServer = tileServer;

	QNetworkRequest request(this->getTileServerUrl(QPoint(0, 0), 0));
	request.setRawHeader("User-Agent", "Qt/SimpleMapView");
	request.setTransferTimeout(5000);

	QNetworkReply* reply = m_networkManager.get(request);

	QEventLoop eventLoop;
	(void)this->connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
	(void)eventLoop.exec();

	if (reply->error() == QNetworkReply::NoError)
	{
		QImage tileImage;
		tileImage.loadFromData(reply->readAll());
		if (tileImage.width() > 0)
		{
			m_tileSize = tileImage.width();
		}
	}
	else
	{
		qDebug() << "[SimpleMapView]" << reply->errorString();
		qDebug() << "[SimpleMapView]" << "failed to set the tile server to" << m_tileServer;
		m_tileServer = SimpleMapView::INVALID_TILE_SERVER;
	}

	reply->deleteLater();

	this->updateMap();
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

QPoint SimpleMapView::calcRequiredTileCount() const
{
	const int x = ceil(((double)this->width()) / m_tileSize);
	const int y = ceil(((double)this->height()) / m_tileSize);

	return QPoint(x, y);
}

QPointF SimpleMapView::geoCoordinateToTilePosition(double latitude, double longitude) const
{
	const double x = ((longitude + 180.0) / (360.0)) * m_tileCountPerAxis;
	const double y = ((1.0 - (log(tan(M_PI_4 + (qDegreesToRadians(latitude) / 2.0))) / M_PI)) / 2.0) * m_tileCountPerAxis;

	return QPointF(x, y);
}

QPointF SimpleMapView::geoCoordinateToTilePosition(const QGeoCoordinate& geoCoordinate) const
{
	return this->geoCoordinateToTilePosition(geoCoordinate.latitude(), geoCoordinate.longitude());
}

QPointF SimpleMapView::geoCoordinateToScreenPosition(double latitude, double longitude) const
{
	return this->tilePositionToScreenPosition(this->geoCoordinateToTilePosition(latitude, longitude));
}

QPointF SimpleMapView::geoCoordinateToScreenPosition(const QGeoCoordinate& geoCoordinate) const
{
	return this->geoCoordinateToScreenPosition(geoCoordinate.latitude(), geoCoordinate.longitude());
}

QGeoCoordinate SimpleMapView::tilePositionToGeoCoordinate(const QPointF& tilePosition) const
{
	const int tileCountPerAxis = (1 << m_zoomLevel);
	const double longitude = tilePosition.x() * (360.0 / tileCountPerAxis) - 180.0;
	const double latitude = qRadiansToDegrees(2.0 * (atan(exp(-(tilePosition.y() * (2.0 / tileCountPerAxis) - 1) * M_PI)) - M_PI_4));

	return QGeoCoordinate(latitude, longitude);
}

QPointF SimpleMapView::tilePositionToScreenPosition(const QPointF& tilePosition) const
{
	const QPointF relativeTilePosition = tilePosition - this->geoCoordinateToTilePosition(m_center);

	const double x = (this->width() / 2.0) + (relativeTilePosition.x() * m_tileSize);
	const double y = (this->height() / 2.0) + (relativeTilePosition.y() * m_tileSize);

	return QPointF(x, y);
}

QPointF SimpleMapView::screenPositionToTilePosition(const QPointF& screenPosition) const
{
	const QPointF centerTilePosition = this->geoCoordinateToTilePosition(m_center);

	const double x = (m_tileSize * (screenPosition.x() - (this->width() / 2.0))) + centerTilePosition.x();
	const double y = (m_tileSize * (screenPosition.y() - (this->height() / 2.0))) + centerTilePosition.y();

	return QPointF(x, y);
}

QGeoCoordinate SimpleMapView::screenPositionToGeoCoordinate(const QPointF& screenPosition) const
{
	return this->tilePositionToGeoCoordinate(this->screenPositionToTilePosition(screenPosition));
}

bool SimpleMapView::validateTilePosition(const QPoint& tilePosition) const
{
	return QRect(0, 0, m_tileCountPerAxis, m_tileCountPerAxis).contains(tilePosition);
}

QString SimpleMapView::getTileKey(const QPoint& tilePosition) const
{
	return QString("%1 %2").arg(tilePosition.x()).arg(tilePosition.y());
}

QPoint SimpleMapView::getTilePosition(const QString& tileKey) const
{
	const QStringList positionStr = tileKey.split(" ");
	const int x = positionStr[0].toInt();
	const int y = positionStr[1].toInt();
	return QPoint(x, y);
}

QUrl SimpleMapView::getTileServerUrl(const QPoint& tilePosition, int zoomLevel) const
{
	QString temp = m_tileServer;
	temp.replace("{x}", QString::number(tilePosition.x()));
	temp.replace("{y}", QString::number(tilePosition.y()));
	temp.replace("{z}", QString::number(zoomLevel));
	return QUrl(temp);
}

void SimpleMapView::updateMap()
{
	if (m_tileServer == SimpleMapView::INVALID_TILE_SERVER) return;

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
	if (noNewTiles) this->update();
}

void SimpleMapView::fetchTile(const QPoint& tilePosition)
{
	if (m_tileServer == SimpleMapView::INVALID_TILE_SERVER) return;

	QNetworkRequest request(this->getTileServerUrl(tilePosition, m_zoomLevel));
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
			}

			reply->deleteLater();
			if (!m_abortingReplies) // m_replyMap will be cleared after abort
			{
				(void)m_replyMap.erase(tileKey);
			}

			if (m_replyMap.size() == 0)
			{
				this->update();
			}
		}
	);
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

std::vector<QString> SimpleMapView::getTilesToRender() const
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
	QPainter paint(this);
	paint.fillRect(event->region().boundingRect(), paint.background());

	for (const auto& tileKey : this->getTilesToRender())
	{
		const QPointF screenPosition = this->tilePositionToScreenPosition(this->getTilePosition(tileKey));
		paint.drawImage(screenPosition.x(), screenPosition.y(), *m_tileMap[tileKey]);
	}

	QWidget::paintEvent(event);
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
		const QPointF centerTilePosition = this->geoCoordinateToTilePosition(m_center);
		const QPoint currentMousePosition = event->pos();
		const QPoint deltaMousePosition = currentMousePosition - m_lastMousePosition;

		// 1.0 / (dx/dlongitude)
		const double deltaLongitude = 360.0 / m_tileCountPerAxis;

		// 1.0 / (dy/dlatitude)
		const double deltaLatitude = -(360.0 / m_tileCountPerAxis) * cos(qDegreesToRadians(this->latitude()));

		this->setCenter(
			(this->latitude() - (deltaMousePosition.y() * (deltaLatitude / m_tileSize))),
			(this->longitude() - (deltaMousePosition.x() * (deltaLongitude / m_tileSize)))
		);

		m_lastMousePosition = currentMousePosition;
	}
}