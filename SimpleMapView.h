#ifndef SIMPLE_MAP_VIEW_H
#define SIMPLE_MAP_VIEW_H

#include "MapMarker.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <array>
#include <QWidget>
#include <QGeoCoordinate>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class SimpleMapView : public QWidget
{
	Q_OBJECT

public:
	explicit SimpleMapView(QWidget* parent = nullptr);
	~SimpleMapView() = default;

public slots:
	void resize(int w, int h);

	int minZoomLevel() const;
	void setMinZoomLevel(int minZoomLevel);

	int maxZoomLevel() const;
	void setMaxZoomLevel(int maxZoomLevel);

	int zoomLevel() const;
	void setZoomLevel(int zoomLevel);

	double latitude() const;
	void setLatitude(double latitude);

	double longitude() const;
	void setLongitude(double longitude);

	const QGeoCoordinate& center() const;
	void setCenter(const QGeoCoordinate& center);
	void setCenter(double latitude, double longitude);

	const QString& tileServer() const;
	void setTileServer(const QString& tileServer);

	bool isZoomLocked() const;
	void lockZoom();
	void unlockZoom();

	bool isGeolocationLocked() const;
	void lockGeolocation();
	void unlockGeolocation();

	bool isMouseWheelZoomEnabled() const;
	void enableMouseWheelZoom();
	void disableMouseWheelZoom();

	bool isMouseMoveMapEnabled() const;
	void enableMouseMoveMap();
	void disableMouseMoveMap();

	MapMarker* addMarker(const QGeoCoordinate& position);
	MapMarker* addMarker(double latitude, double longitude);
	void removeMarker(MapMarker* marker);
	void clearMarkers();
	std::vector<MapMarker*> markers() const;

signals:
	void zoomLevelChanged();
	void centerChanged();
	void tileServerChanged();

protected:
	QPoint calcRequiredTileCount() const;

	QPointF geoCoordinateToTilePosition(double latitude, double longitude) const;
	QPointF geoCoordinateToTilePosition(const QGeoCoordinate& geoCoordinate) const;
	QPointF geoCoordinateToScreenPosition(double latitude, double longitude) const;
	QPointF geoCoordinateToScreenPosition(const QGeoCoordinate& geoCoordinate) const;

	QGeoCoordinate tilePositionToGeoCoordinate(const QPointF& tilePosition) const;
	QPointF tilePositionToScreenPosition(const QPointF& tilePosition) const;

	QPointF screenPositionToTilePosition(const QPointF& screenPosition) const;
	QGeoCoordinate screenPositionToGeoCoordinate(const QPointF& screenPosition) const;

	bool validateTilePosition(const QPoint& tilePosition) const;

	QString getTileKey(const QPoint& tilePosition) const;
	QPoint getTilePosition(const QString& tileKey) const;
	QUrl getTileServerUrl(const QPoint& tilePosition, int zoomLevel) const;

	void updateMap();
	void fetchTile(const QPoint& tilePosition);
	void abortReplies();
	
	std::vector<QString> visibleTiles() const;

	virtual void paintEvent(QPaintEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	QPainterPath calcPaintClipRegion() const;
	std::array<int, 4> extractBorderRadiiFromStyleSheet() const; // top-left, top-right, bottom-right, bottom-left
	QPen extractBorderPenFromStyleSheet() const;

private:
	int m_zoomLevel;
	int m_minZoomLevel;
	int m_maxZoomLevel;
	int m_tileCountPerAxis; // pow(2, m_zoomLevel)

	QGeoCoordinate m_center;

	QString m_tileServer;
	QNetworkAccessManager m_networkManager;
	int m_tileSize;
	bool m_abortingReplies;

	bool m_lockZoom;
	bool m_lockGeolocation;
	bool m_disableMouseWheelZoom;
	bool m_disableMouseMoveMap;

	QPoint m_lastMousePosition;

	std::unordered_map<QString, QNetworkReply*> m_replyMap;
	std::unordered_map<QString, std::unique_ptr<QImage>> m_tileMap;

public:
	class TileServers
	{
	public:
		TileServers() = delete;
		TileServers(const TileServers&) = delete;
		TileServers& operator=(const TileServers&) = delete;

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
};

#endif