#ifndef SIMPLE_MAP_VIEW_H
#define SIMPLE_MAP_VIEW_H

#include "MapItem.h"
#include "MapEllipse.h"
#include "MapRect.h"
#include "MapText.h"
#include "MapImage.h"
#include "MapLines.h"
#include "MapPolygon.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <array>
#include <QWidget>
#include <QGeoCoordinate>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QTimer>

class SimpleMapView : public QWidget
{
	Q_OBJECT

public:
	explicit SimpleMapView(QWidget* parent = nullptr);
	~SimpleMapView() = default;

public slots:
	void resize(int w, int h);

	/** Gets the minimum zoom level. */
	int minZoomLevel() const;
	/** Sets the minimum zoom level. */
	void setMinZoomLevel(int minZoomLevel);

	/** Gets the maximum zoom level. */
	int maxZoomLevel() const;
	/** Sets the maximum zoom level. */
	void setMaxZoomLevel(int maxZoomLevel);

	/** Gets the current zoom level. */
	int zoomLevel() const;
	/** Sets the current zoom level. */
	void setZoomLevel(int zoomLevel);

	/** Gets the center latitude. */
	qreal latitude() const;
	/** Sets the center latitude. */
	void setLatitude(qreal latitude);

	/** Gets the center longitude. */
	qreal longitude() const;
	/** Sets the center longitude. */
	void setLongitude(qreal longitude);

	/** Gets the center coordinates. */
	const QGeoCoordinate& center() const;
	/** Sets the center coordinates. */
	void setCenter(const QGeoCoordinate& center);
	/** Sets the center coordinates. */
	void setCenter(qreal latitude, qreal longitude);

	/** Gets the used tile server. */
	const QString& tileServer() const;
	/** Sets the tile server. */
	void setTileServer(const QString& tileServer, bool wait = true);
	/** Sets the first available tile server and adds the remaining as backup. */
	void setTileServer(const std::vector<QString>& tileServers);

	/** Gets the backup server list. */
	const std::vector<QString>& backupTileServers() const;
	/** Adds a tile server to the backup server list. */
	void addBackupTileServer(const QString& tileServer);
	/** Adds the tile servers to the backup server list. */
	void addBackupTileServer(const std::vector<QString>& tileServers);
	/** Clears the backup tile server list. */
	void clearBackupTileServers();

	/** Checks whether the zoom is locked to the current level. */
	bool isZoomLocked() const;
	/** Locks the zoom to the current level so it cannot be changed. */
	void lockZoom();
	/** Unlocks the zoom level so it can be changed. */
	void unlockZoom();

	/** Checks whether the geolocation is locked. */
	bool isGeolocationLocked() const;
	/** Locks the geolocation so it cannot be changed. */
	void lockGeolocation();
	/** Unlocks the geolocation so it can be changed. */
	void unlockGeolocation();

	/** Checks whether changing the zoom level via mouse wheel is enabled. */
	bool isMouseWheelZoomEnabled() const;
	/** Enables changing the zoom level via mouse wheel. */
	void enableMouseWheelZoom();
	/** Disables changing the zoom level via mouse wheel. */
	void disableMouseWheelZoom();

	/** Checks whether moving the map via mouse is enabled. */
	bool isMouseMoveMapEnabled() const;
	/** Enables moving the map via mouse. */
	void enableMouseMoveMap();
	/** Disables moving the map via mouse. */
	void disableMouseMoveMap();

	/** Gets the icon used for markers. */
	const QImage& markerIcon() const;
	/** Sets the icon used for markers. */
	void setMarkerIcon(const QImage& icon);

	/** Adds a new marker to the map at the provided geolocation. */
	MapImage* addMarker(qreal latitude, qreal longitude);
	/** Adds a new marker to the map at the provided geolocation. */
	MapImage* addMarker(const QGeoCoordinate& position = QGeoCoordinate());

	/**
	 * Downloads tiles and saves them for offline use.
	 * 
	 * @param path Path to save the tiles.
	 * @param p1 First geocoordinate of the rectangular region.
	 * @param p2 Second geocoordinate of the rectangular region.
	 * @param z1 First zoom level.
	 * @param z2 Second zoom level.
	 */
	void downloadTiles(const QString& path, const QGeoCoordinate& p1, const QGeoCoordinate& p2, int z1, int z2);

	/** Converts the geocoordinates to tile position used in the tile servers. */
	QPointF geoCoordinateToTilePosition(qreal latitude, qreal longitude) const;
	/** Converts the geocoordinates to tile position used in the tile servers. */
	QPointF geoCoordinateToTilePosition(const QGeoCoordinate& geoCoordinate) const;
	/** Converts the geocoordinates to screen position in pixels. */
	QPointF geoCoordinateToScreenPosition(qreal latitude, qreal longitude) const;
	/** Converts the geocoordinates to screen position in pixels. */
	QPointF geoCoordinateToScreenPosition(const QGeoCoordinate& geoCoordinate) const;

	/** Converts the tile position to geocoordinates. */
	QGeoCoordinate tilePositionToGeoCoordinate(const QPointF& tilePosition) const;
	/** Converts the tile position to screen position in pixels. */
	QPointF tilePositionToScreenPosition(const QPointF& tilePosition) const;

	/** Converts the screen position in pixels to tile position. */
	QPointF screenPositionToTilePosition(const QPointF& screenPosition) const;
	/** Converts the screen position in pixels to geocoordinates. */
	QGeoCoordinate screenPositionToGeoCoordinate(const QPointF& screenPosition) const;

signals:
	/** A signal that's triggered when the zoom level changes. */
	void zoomLevelChanged();
	/** A signal that's triggered when the center coordinate changes. */
	void centerChanged();
	/** A signal that's triggered when the tile server changes. */
	void tileServerChanged();

protected:
	/** Calculates the number of tiles required for rendering the map to the screen. */
	QPoint calcRequiredTileCount() const;

	/** Checks whether the tile is required and should be fetched for rendering. */
	virtual bool validateTilePosition(const QPoint& tilePosition) const;

	/** Converts the tile position to tile key. */
	virtual QString getTileKey(const QPoint& tilePosition) const;
	/** Converts tile key to tile position. */
	virtual QPoint getTilePosition(const QString& tileKey, int* outZoomLevel = nullptr) const;
	/** Creates a valid tile server URL. */
	virtual QUrl formatTileServerUrl(QString tileServerUrl, const QPoint& tilePosition, int zoomLevel) const;

	/** Fetches the required tiles from the server and updates the display. */
	void updateMap();
	/** Fetches the tile from the server. */
	void fetchTile(const QPoint& tilePosition);
	/** Aborts all ongoing requests and drops the replies. */
	void abortReplies();

	/** Gets all visible tiles. */
	std::vector<QString> visibleTiles() const;

	/** Renders all tiles and map items to the display. */
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	void checkTileServers();
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

	std::vector<QString> m_backupTileServers;
	QTimer m_tileServerTimer; // tries to connect to the current server, or one of the backup servers, periodically.
	size_t m_backupTileServerIndex;

	bool m_lockZoom;
	bool m_lockGeolocation;
	bool m_disableMouseWheelZoom;
	bool m_disableMouseMoveMap;

	QPoint m_lastMousePosition;
	
	QImage m_markerIcon;

	std::unordered_map<QString, QNetworkReply*> m_replyMap;
	std::unordered_map<QString, std::unique_ptr<QImage>> m_tileMap;

	static constexpr unsigned int TILE_SERVER_TIMER_INTERVAL_MS = 100;

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