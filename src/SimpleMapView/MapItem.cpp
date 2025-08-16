#include "SimpleMapView/MapItem.h"
#include "SimpleMapView.h"

MapPoint::MapPoint()
	: MapPoint(QPointF())
{
}

MapPoint::MapPoint(const QPointF& screenPoint)
{
	m_val.setValue(screenPoint);
}

MapPoint::MapPoint(const QGeoCoordinate& geoPoint)
{
	m_val.setValue(geoPoint);
}

MapPoint& MapPoint::operator=(const QPointF& screenPoint) 
{
	m_val.setValue(screenPoint);
	return *this;
}

MapPoint& MapPoint::operator=(const QGeoCoordinate& geoPoint) 
{
	m_val.setValue(geoPoint);
	return *this;
}

bool MapPoint::isValid() const
{
	if (m_val.canConvert<QPointF>())
		return !m_val.value<QPointF>().isNull();
	return m_val.value<QGeoCoordinate>().isValid();
}

QPointF MapPoint::screenPoint(const SimpleMapView* map) const
{
	if (m_val.canConvert<QPointF>())
	{
		return m_val.value<QPointF>();
	}

	if (map != nullptr)
	{
		return map->geoCoordinateToScreenPosition(m_val.value<QGeoCoordinate>());
	}
	return QPointF();
}

QGeoCoordinate MapPoint::geoPoint(const SimpleMapView* map) const
{
	if (m_val.canConvert<QGeoCoordinate>())
	{
		return m_val.value<QGeoCoordinate>();
	}

	if (map != nullptr)
	{
		return map->screenPositionToGeoCoordinate(m_val.value<QPointF>());
	}
	return QGeoCoordinate();
}

MapSize::MapSize()
	: MapSize(QSizeF())
{
}

MapSize::MapSize(const QSizeF& screenSize)
{
	m_val.setValue(screenSize);
}

MapSize::MapSize(const QGeoCoordinate& geoSize)
{
	m_val.setValue(geoSize);
}

MapSize& MapSize::operator=(const QPointF& screenSize) 
{
	m_val.setValue(screenSize);
	return *this;
}

MapSize& MapSize::operator=(const QGeoCoordinate& geoSize) 
{
	m_val.setValue(geoSize);
	return *this;
}

bool MapSize::isValid() const
{
	if (m_val.canConvert<QSizeF>())
		return m_val.value<QSizeF>().isValid();
	return m_val.value<QGeoCoordinate>().isValid();
}

QSizeF MapSize::screenSize(const SimpleMapView* map, const MapPoint& topLeft) const
{
	if (m_val.canConvert<QSizeF>())
	{
		return m_val.value<QSizeF>();
	}

	if (map != nullptr)
	{
		QGeoCoordinate geoPoint = topLeft.geoPoint(map);
		geoPoint.setLatitude(geoPoint.latitude() + m_val.value<QGeoCoordinate>().latitude());
		geoPoint.setLongitude(geoPoint.longitude() + m_val.value<QGeoCoordinate>().longitude());

		const QPointF result = map->geoCoordinateToScreenPosition(geoPoint) - topLeft.screenPoint(map);
		return QSizeF(result.x(), result.y());
	}
	return QSizeF();
}

QGeoCoordinate MapSize::geoSize(const SimpleMapView* map, const MapPoint& topLeft) const
{
	if (m_val.canConvert<QGeoCoordinate>())
	{
		return m_val.value<QGeoCoordinate>();
	}

	if (map != nullptr)
	{
		const QPointF topLeftScreenPoint = topLeft.screenPoint(map);
		const QPointF screenPoint = QPointF(
			(topLeftScreenPoint.x() + m_val.value<QSizeF>().width()),
			(topLeftScreenPoint.y() + m_val.value<QSizeF>().height())
		);

		return map->screenPositionToGeoCoordinate(screenPoint);
	}
	return QGeoCoordinate();
}

MapItem::MapItem(QObject* parent)
	: QObject(parent),
	m_pen(Qt::black, 0.0)
{
}

const QPen& MapItem::pen() const
{
	return m_pen;
}

void MapItem::setPen(const QPen& pen)
{
	m_pen = pen;

	this->repaintMap();

	emit this->changed();
	emit this->penChanged();
}

QColor MapItem::penColor() const
{
	return m_pen.color();
}

void MapItem::setPenColor(const QColor& color)
{
	m_pen.setColor(color);
}

qreal MapItem::penWidth() const
{
	return m_pen.widthF();
}

void MapItem::setPenWidth(qreal width)
{
	m_pen.setWidthF(width);
}

Qt::PenStyle MapItem::penStyle() const
{
	return m_pen.style();
}

void MapItem::setPenStyle(Qt::PenStyle style)
{
	m_pen.setStyle(style);
}

SimpleMapView* MapItem::getMapView() const
{
	QObject* parent = this->parent();
	while (parent != nullptr)
	{
		if (parent->inherits("SimpleMapView"))
		{
			return (SimpleMapView*)parent;
		}
		parent = parent->parent();
	}
	return nullptr;
}

void MapItem::repaintMap()
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		map->repaint();
	}
}