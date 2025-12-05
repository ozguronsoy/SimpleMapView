#include "SimpleMapView/utils.h"
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

TileServers::TileServers(QObject* parent) : QObject(parent) {}

QString TileServers::Invalid() const { return TileServers::INVALID; }
QString TileServers::Osm() const { return TileServers::OSM; }
QString TileServers::OpenTopoMap() const { return TileServers::OPENTOPOMAP; }
QString TileServers::GoogleMap() const { return TileServers::GOOGLE_MAP; }
QString TileServers::GoogleSat() const { return TileServers::GOOGLE_SAT; }
QString TileServers::GoogleLand() const { return TileServers::GOOGLE_LAND; }
QString TileServers::CartoDbPositron() const { return TileServers::CARTODB_POSITRON; }
QString TileServers::CartoDbDarkMatter() const { return TileServers::CARTODB_DARK_MATTER; }
QString TileServers::ThunderforestTransport() const { return TileServers::THUNDERFOREST_TRANSPORT; }
QString TileServers::ThunderforestLandscape() const { return TileServers::THUNDERFOREST_LANDSCAPE; }
QString TileServers::ThunderforestOutdoors() const { return TileServers::THUNDERFOREST_OUTDOORS; }
QString TileServers::EsriWorldStreetMap() const { return TileServers::ESRI_WORLD_STREET_MAP; }
QString TileServers::EsriWorldImagery() const { return TileServers::ESRI_WORLD_IMAGERY; }

#ifdef SIMPLE_MAP_VIEW_USE_QML

MapPoint SimpleMapViewQmlHelpers::createMapPoint(const QPointF& screenPoint) { return MapPoint(screenPoint); }
MapPoint SimpleMapViewQmlHelpers::createMapPoint(const QGeoCoordinate& geoPoint) { return MapPoint(geoPoint); }
MapSize SimpleMapViewQmlHelpers::createMapSize(const QSizeF& screenSize) { return MapSize(screenSize); }
MapSize SimpleMapViewQmlHelpers::createMapSize(const QGeoCoordinate& geoSize) { return MapSize(geoSize); }

QObject* SimpleMapViewQmlHelpers::findChild(QObject* parent, const QString& className)
{
    if (parent)
        for (QObject* child : parent->children())
            if (QString(child->metaObject()->className()).contains(className))
                return child;
    return nullptr;
}

#endif
