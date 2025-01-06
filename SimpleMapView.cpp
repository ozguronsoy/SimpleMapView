#include "SimpleMapView.h"
#include <algorithm>
#include <cmath>
#include <QEventLoop>
#include <QPainter>
#include <QWheelEvent>

SimpleMapView::SimpleMapView(QWidget* parent)
	: QWidget(parent),
	m_minZoomLevel(0),
	m_maxZoomLevel(21),
	m_zoomLevel(17),
	m_tileCountPerAxis(1 << m_zoomLevel),
	m_center(39.912341799204775, 32.851170267919244),
	m_networkManager(this),
	m_nTilesToFetch(0),
	m_fetchedTileCount(0),
	m_tileSize(256),
	m_abortingReplies(false)
{
	this->setTileServer("https://mt0.google.com/vt/lyrs=m&hl=en&x={x}&y={y}&z={z}&s=Ga");
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
}

int SimpleMapView::zoomLevel() const
{
	return m_zoomLevel;
}

void SimpleMapView::setZoomLevel(int zoomLevel)
{
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

	// maybe add validation
	m_tileServer = tileServer;

	// find tile size
	{
		QNetworkRequest request(this->getTileServerUrl(QPoint(0, 0), 0));
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
			// TODO error handling
			qDebug() << "SimpleMapView: " << reply->errorString();
		}

		reply->deleteLater();
	}

	this->updateMap();
}

QSize SimpleMapView::calcRequiredTileCount() const
{
	const int x = std::ceil(((double)this->width()) / m_tileSize);
	const int y = std::ceil(((double)this->height()) / m_tileSize);

	return QSize(x, y);
}

QPointF SimpleMapView::calcCenterTilePosition() const
{
	const double x = ((this->longitude() + 180.0) / (360.0)) * m_tileCountPerAxis;
	const double y = (1.0 - (std::log(std::tan(M_PI_4 + (this->latitude() * M_PI / 360.0))) / M_PI)) * 0.5 * m_tileCountPerAxis;

	return QPointF(x, y);
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
	const QSize requiredTileCount = this->calcRequiredTileCount();
	const QPointF centerTilePosition = this->calcCenterTilePosition();

	const int x_start = (-requiredTileCount.width() / 2) - 1;
	const int x_end = (requiredTileCount.width() / 2) + 1;
	const int y_start = (-requiredTileCount.height() / 2) - 1;
	const int y_end = (requiredTileCount.height() / 2) + 1;

	m_fetchedTileCount = 0;
	m_nTilesToFetch = 0;

	// find the tiles that will be fetched
	std::vector<QPoint> tilePositions;
	tilePositions.reserve((x_end - x_start + 1) * (y_end - y_start + 1));

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
				tilePositions.push_back(tilePosition);
				m_nTilesToFetch++;
			}
		}
	}

	// if the map is moved but no new tiles are required, update the widget
	if (m_nTilesToFetch == 0)
	{
		this->update();
		return;
	}

	for (const QPoint& tilePosition : tilePositions)
	{
		this->fetchTile(tilePosition);
	}
}

void SimpleMapView::fetchTile(const QPoint& tilePosition)
{
	QNetworkRequest request(this->getTileServerUrl(tilePosition, m_zoomLevel));
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
				// TODO error handling
				qDebug() << "SimpleMapView: " << reply->errorString();
			}

			reply->deleteLater();
			if (!m_abortingReplies) // m_replyMap will be cleared after abort
			{
				(void)m_replyMap.erase(tileKey);
			}

			m_fetchedTileCount++;
			if (m_fetchedTileCount == m_nTilesToFetch)
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

void SimpleMapView::paintEvent(QPaintEvent* event)
{
	QPainter paint(this);
	paint.fillRect(event->region().boundingRect(), paint.background());

	const QSize requiredTileCount = this->calcRequiredTileCount();
	const QPointF centerTilePosition = this->calcCenterTilePosition();

	const int w_2 = this->width() / 2;
	const int h_2 = this->height() / 2;
	const int tw = m_tileSize;
	const int th = m_tileSize;

	for (const auto& tile : m_tileMap)
	{
		const QPoint p = this->getTilePosition(tile.first);

		paint.drawImage(
			w_2 + ((p.x() - centerTilePosition.x()) * tw),
			h_2 + ((p.y() - centerTilePosition.y()) * th),
			*tile.second
		);
	}

	QWidget::paintEvent(event);
}

void SimpleMapView::wheelEvent(QWheelEvent* event)
{
	const int angleDelta = event->angleDelta().y();
	if (angleDelta > 0)
		this->setZoomLevel(this->zoomLevel() + 1);
	else if (angleDelta < 0)
		this->setZoomLevel(this->zoomLevel() - 1);

	QWidget::wheelEvent(event);
}

void SimpleMapView::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		m_lastMousePosition = event->pos();
	}

	QWidget::mousePressEvent(event);
}

void SimpleMapView::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		constexpr double latitudeSpeed = 180.0 / 167.0;
		constexpr double longitudeSpeed = 360.0 / 256.0;

		const QPoint currentMousePosition = event->pos();
		const QPoint deltaMousePosition = currentMousePosition - m_lastMousePosition;

		const double deltaLatitude = latitudeSpeed / m_tileCountPerAxis;
		const double deltaLongitude = longitudeSpeed / m_tileCountPerAxis;

		this->setCenter(this->latitude() + deltaMousePosition.y() * deltaLatitude, this->longitude() - deltaMousePosition.x() * deltaLongitude);

		m_lastMousePosition = currentMousePosition;
	}

	QWidget::mouseMoveEvent(event);
}