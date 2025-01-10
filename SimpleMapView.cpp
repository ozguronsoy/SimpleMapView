#include "SimpleMapView.h"
#include <algorithm>
#include <cmath>
#include <QRegularExpression>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QWheelEvent>
#include <QDebug>

#define GET_IN_RANGE(v, minv, maxv) (std::min(std::max((v), (minv)), (maxv)))

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

	emit this->tileServerChanged();
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

MapMarker* SimpleMapView::addMarker(const QGeoCoordinate& position)
{
	return this->addMarker(position.latitude(), position.longitude());
}

MapMarker* SimpleMapView::addMarker(double latitude, double longitude)
{
	std::unique_ptr<MapMarker>& marker = m_markers.emplace_back(new MapMarker(latitude, longitude));

	auto repaintMap = [this]() { this->repaint(); };

	(void)marker->connect(marker.get(), &MapMarker::positionChanged, repaintMap);
	(void)marker->connect(marker.get(), &MapMarker::labelChanged, repaintMap);
	(void)marker->connect(marker.get(), &MapMarker::iconChanged, repaintMap);

	return marker.get();
}

void SimpleMapView::removeMarker(MapMarker* marker)
{
	m_markers.remove_if([marker](const std::unique_ptr<MapMarker>& m) { return m.get() == marker; });
}

std::vector<MapMarker*> SimpleMapView::markers() const
{
	std::vector<MapMarker*> result;
	result.reserve(m_markers.size());

	for (const std::unique_ptr<MapMarker>& marker : m_markers)
	{
		result.push_back(marker.get());
	}

	return result;
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
	paint.setRenderHint(QPainter::Antialiasing);
	paint.setRenderHint(QPainter::LosslessImageRendering);
	paint.setRenderHint(QPainter::TextAntialiasing);

	const QPainterPath painterPath = this->calcPaintClipRegion();
	paint.setClipPath(painterPath);

	// fill background
	paint.fillRect(event->region().boundingRect(), paint.background());

	for (const auto& tileKey : this->getTilesToRender())
	{
		const QPointF screenPosition = this->tilePositionToScreenPosition(this->getTilePosition(tileKey));
		paint.drawImage(screenPosition.x(), screenPosition.y(), *m_tileMap[tileKey]);
	}

	// draw border
	paint.setPen(this->extractBorderPenFromStyleSheet());
	paint.drawPath(painterPath);

	// draw markers
	for (const std::unique_ptr<MapMarker>& marker : m_markers)
	{
		const QPointF markerScreenPosition = this->geoCoordinateToScreenPosition(marker->position());
		const QImage markerIcon = marker->icon().scaled(marker->iconSize().width(), marker->iconSize().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

		const double mx = (markerScreenPosition.x() - (markerIcon.width() / 2.0));
		const double my = (markerScreenPosition.y() - markerIcon.height());

		paint.drawImage(mx, my, markerIcon);

		if (marker->hasLabel())
		{
			paint.setFont(marker->labelFont());
			paint.setPen(marker->labelColor());

			QFontMetricsF fontMetrics(marker->labelFont());
			const QRectF fontRect = fontMetrics.boundingRect(marker->label());

			paint.drawText(
				(markerScreenPosition.x() - (fontRect.width() / 2.0)),
				(my - (fontRect.height() / 2.0)),
				marker->label()
			);
		}
	}
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
			for (const QString s : value.split(" "))
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
			for (const QString s : value.split(" "))
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
			for (const QString s : value.split(" "))
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