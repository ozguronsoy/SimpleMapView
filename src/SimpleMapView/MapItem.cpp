#include "SimpleMapView/MapItem.h"
#include "SimpleMapView.h"

MapPoint::MapPoint()
	: MapPoint(QPointF())
{
}

MapPoint::MapPoint(const QPointF& screenPoint)
	: QVariant(screenPoint)
{
}

MapPoint::MapPoint(const QGeoCoordinate& geoPoint)
	: QVariant()
{
	this->setValue(geoPoint);
}

MapPoint& MapPoint::operator=(const QPointF& screenPoint) 
{
	this->setValue(screenPoint);
	return *this;
}

MapPoint& MapPoint::operator=(const QGeoCoordinate& geoPoint) 
{
	this->setValue(geoPoint);
	return *this;
}

bool MapPoint::isValid() const
{
	if (this->canConvert<QPointF>())
		return !this->value<QPointF>().isNull();
	return this->value<QGeoCoordinate>().isValid();
}

QPointF MapPoint::screenPoint(const SimpleMapView* map) const
{
	if (this->canConvert<QPointF>())
	{
		return this->value<QPointF>();
	}

	if (map != nullptr)
	{
		return map->geoCoordinateToScreenPosition(this->value<QGeoCoordinate>());
	}
	return QPointF();
}

QGeoCoordinate MapPoint::geoPoint(const SimpleMapView* map) const
{
	if (this->canConvert<QGeoCoordinate>())
	{
		return this->value<QGeoCoordinate>();
	}

	if (map != nullptr)
	{
		return map->screenPositionToGeoCoordinate(this->value<QPointF>());
	}
	return QGeoCoordinate();
}

MapSize::MapSize()
	: MapSize(QSizeF())
{
}

MapSize::MapSize(const QSizeF& screenSize)
	: QVariant(screenSize)
{
}

MapSize::MapSize(const QGeoCoordinate& geoSize)
	: QVariant()
{
	this->setValue(geoSize);
}

MapSize& MapSize::operator=(const QPointF& screenSize) 
{
	this->setValue(screenSize);
	return *this;
}

MapSize& MapSize::operator=(const QGeoCoordinate& geoSize) 
{
	this->setValue(geoSize);
	return *this;
}

bool MapSize::isValid() const
{
	if (this->canConvert<QSizeF>())
		return this->value<QSizeF>().isValid();
	return this->value<QGeoCoordinate>().isValid();
}

QSizeF MapSize::screenSize(const SimpleMapView* map, const MapPoint& topLeft) const
{
	if (this->canConvert<QSizeF>())
	{
		return this->value<QSizeF>();
	}

	if (map != nullptr)
	{
		QGeoCoordinate geoPoint = topLeft.geoPoint(map);
		geoPoint.setLatitude(geoPoint.latitude() + this->value<QGeoCoordinate>().latitude());
		geoPoint.setLongitude(geoPoint.longitude() + this->value<QGeoCoordinate>().longitude());

		const QPointF result = map->geoCoordinateToScreenPosition(geoPoint) - topLeft.screenPoint(map);
		return QSizeF(result.x(), result.y());
	}
	return QSizeF();
}

QGeoCoordinate MapSize::geoSize(const SimpleMapView* map, const MapPoint& topLeft) const
{
	if (this->canConvert<QGeoCoordinate>())
	{
		return this->value<QGeoCoordinate>();
	}

	if (map != nullptr)
	{
		const QPointF topLeftScreenPoint = topLeft.screenPoint(map);
		const QPointF screenPoint = QPointF(
			(topLeftScreenPoint.x() + this->value<QSizeF>().width()),
			(topLeftScreenPoint.y() + this->value<QSizeF>().height())
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