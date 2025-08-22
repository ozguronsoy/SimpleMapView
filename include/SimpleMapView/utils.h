#ifndef SIMPLE_MAP_VIEW_UTILS_H
#define SIMPLE_MAP_VIEW_UTILS_H

#include <QPoint>
#include <QSize>
#include <QGeoCoordinate>
#include <QVariant>
#include <QString>
#include <QMetaType>

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QtQml/qqmlregistration.h>
#include <QSGNode>

using MapRenderer = QSGNode;

#else

#include <QPainter>
using MapRenderer = QPainter;


#endif

class SimpleMapView;

/**
 * @brief Represents a point on a map, which can be stored either in screen coordinates (pixels) or geographic coordinates (degrees).
 */
class MapPoint
{
	Q_GADGET;

public:
	MapPoint();
	MapPoint(const QPointF& screenPoint);
	MapPoint(const QGeoCoordinate& geoPoint);

	MapPoint& operator=(const QPointF& screenPoint);
	MapPoint& operator=(const QGeoCoordinate& geoPoint);

	bool isValid() const;
	QPointF screenPoint(const SimpleMapView* map) const;
	QGeoCoordinate geoPoint(const SimpleMapView* map) const;

private:
	QVariant m_val;
};

/**
 * @brief Represents a size on a map, which can be expressed in screen units (pixels) or geographic units (degrees).
 */
class MapSize
{
	Q_GADGET;

public:
	MapSize();
	MapSize(const QSizeF& screenSize);
	MapSize(const QGeoCoordinate& geoSize);

	MapSize& operator=(const QPointF& screenSize);
	MapSize& operator=(const QGeoCoordinate& geoSize);

	bool isValid() const;
	QSizeF screenSize(const SimpleMapView* map, const MapPoint& topLeft) const;
	QGeoCoordinate geoSize(const SimpleMapView* map, const MapPoint& topLeft) const;

private:
	QVariant m_val;
};

Q_DECLARE_METATYPE(MapPoint);
Q_DECLARE_METATYPE(MapSize);

/**
 * @brief Defines tile server URI string constants.
 */
class TileServers : public QObject
{
	Q_OBJECT;
	Q_PROPERTY(QString INVALID READ Invalid CONSTANT);
	Q_PROPERTY(QString OSM READ Osm CONSTANT);
	Q_PROPERTY(QString OPENTOPOMAP READ OpenTopoMap CONSTANT);
	Q_PROPERTY(QString GOOGLE_MAP READ GoogleMap CONSTANT);
	Q_PROPERTY(QString GOOGLE_SAT READ GoogleSat CONSTANT);
	Q_PROPERTY(QString GOOGLE_LAND READ GoogleLand CONSTANT);
	Q_PROPERTY(QString CARTODB_POSITRON READ CartoDbPositron CONSTANT);
	Q_PROPERTY(QString CARTODB_DARK_MATTER READ CartoDbDarkMatter CONSTANT);
	Q_PROPERTY(QString THUNDERFOREST_TRANSPORT READ ThunderforestTransport CONSTANT);
	Q_PROPERTY(QString THUNDERFOREST_LANDSCAPE READ ThunderforestLandscape CONSTANT);
	Q_PROPERTY(QString THUNDERFOREST_OUTDOORS READ ThunderforestOutdoors CONSTANT);
	Q_PROPERTY(QString ESRI_WORLD_STREET_MAP READ EsriWorldStreetMap CONSTANT);
	Q_PROPERTY(QString ESRI_WORLD_IMAGERY READ EsriWorldImagery CONSTANT);

#ifdef SIMPLE_MAP_VIEW_USE_QML
	QML_ELEMENT;
#endif

public:
	explicit TileServers(QObject* parent = nullptr);

	QString Invalid() const;
	QString Osm() const;
	QString OpenTopoMap() const;
	QString GoogleMap() const;
	QString GoogleSat() const;
	QString GoogleLand() const;
	QString CartoDbPositron() const;
	QString CartoDbDarkMatter() const;
	QString ThunderforestTransport() const;
	QString ThunderforestLandscape() const;
	QString ThunderforestOutdoors() const;
	QString EsriWorldStreetMap() const;
	QString EsriWorldImagery() const;

	static constexpr const char* INVALID = "tile_server_invalid";
	static constexpr const char* OSM = "https://tile.openstreetmap.org/{z}/{x}/{y}.png";
	static constexpr const char* OPENTOPOMAP = "https://tile.opentopomap.org/{z}/{x}/{y}.png";
	static constexpr const char* GOOGLE_MAP = "https://mt0.google.com/vt/lyrs=m&hl=en&x={x}&y={y}&z={z}&s=Ga";
	static constexpr const char* GOOGLE_SAT = "https://mt0.google.com/vt/lyrs=y&hl=en&x={x}&y={y}&z={z}&s=Ga";
	static constexpr const char* GOOGLE_LAND = "https://mt0.google.com/vt/lyrs=p&hl=en&x={x}&y={y}&z={z}&s=Ga";
	static constexpr const char* CARTODB_POSITRON = "https://a.basemaps.cartocdn.com/light_all/{z}/{x}/{y}.png";
	static constexpr const char* CARTODB_DARK_MATTER = "https://a.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}.png";
	static constexpr const char* THUNDERFOREST_TRANSPORT = "https://tile.thunderforest.com/transport/{z}/{x}/{y}.png";
	static constexpr const char* THUNDERFOREST_LANDSCAPE = "https://tile.thunderforest.com/landscape/{z}/{x}/{y}.png";
	static constexpr const char* THUNDERFOREST_OUTDOORS = "https://tile.thunderforest.com/outdoors/{z}/{x}/{y}.png";
	static constexpr const char* ESRI_WORLD_STREET_MAP = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Street_Map/MapServer/tile/{z}/{y}/{x}";
	static constexpr const char* ESRI_WORLD_IMAGERY = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}";
};

#ifdef SIMPLE_MAP_VIEW_USE_QML

class SimpleMapViewQmlHelpers final : public QObject
{
	Q_OBJECT;

public:
	Q_INVOKABLE static MapPoint createMapPoint(const QPointF& screenPoint);
	Q_INVOKABLE static MapPoint createMapPoint(const QGeoCoordinate& geoPoint);
	Q_INVOKABLE static MapSize createMapSize(const QSizeF& screenSize);
	Q_INVOKABLE static MapSize createMapSize(const QGeoCoordinate& geoSize);
};

#endif

#endif