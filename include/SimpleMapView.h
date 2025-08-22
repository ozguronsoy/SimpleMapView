#ifndef SIMPLE_MAP_VIEW_H
#define SIMPLE_MAP_VIEW_H

#include "SimpleMapView/utils.h"
#include "SimpleMapView/MapItem.h"
#include "SimpleMapView/MapEllipse.h"
#include "SimpleMapView/MapRect.h"
#include "SimpleMapView/MapText.h"
#include "SimpleMapView/MapImage.h"
#include "SimpleMapView/MapLines.h"
#include "SimpleMapView/MapPolygon.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <array>
#include <QGeoCoordinate>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVector>
#include <QTimer>

#ifndef SIMPLE_MAP_VIEW_USE_QML

#include <QWidget>
using SimpleMapViewBase = QWidget;

#else

#include <QQuickItem>
using SimpleMapViewBase = QQuickItem;

#endif

/**
 * @brief A widget for displaying tile based maps.
 */
class SimpleMapView : public SimpleMapViewBase
{
	Q_OBJECT;
	Q_PROPERTY(int zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged);
	Q_PROPERTY(int minZoomLevel READ minZoomLevel WRITE setMinZoomLevel NOTIFY zoomLevelChanged);
	Q_PROPERTY(int maxZoomLevel READ maxZoomLevel WRITE setMaxZoomLevel NOTIFY zoomLevelChanged);
	Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged);
	Q_PROPERTY(qreal latitude READ latitude WRITE setLatitude NOTIFY centerChanged);
	Q_PROPERTY(qreal longitude READ longitude WRITE setLongitude NOTIFY centerChanged);
	Q_PROPERTY(QString tileServer READ tileServer WRITE setTileServer NOTIFY tileServerChanged);
	Q_PROPERTY(QVector<QString> backupTileServers READ backupTileServers WRITE addBackupTileServer NOTIFY tileServerChanged);
	Q_PROPERTY(bool lockZoom READ isZoomLocked WRITE setLockZoom);
	Q_PROPERTY(bool lockGeolocation READ isGeolocationLocked WRITE setLockGeolocation);
	Q_PROPERTY(bool disableMouseWheelZoom READ isMouseWheelZoomDisabled WRITE setDisableMouseWheelZoom);
	Q_PROPERTY(bool disableMouseMoveMap READ isMouseMoveMapDisabled WRITE setDisableMouseMoveMap);
	Q_PROPERTY(QString markerIcon WRITE setMarkerIcon);

#ifdef SIMPLE_MAP_VIEW_USE_QML
	QML_ELEMENT;
#endif

public:
	/** Represents the source location from which map tiles are loaded. */
	enum class TileServerSource
	{
		/** Invalid source. */
		Invalid,
		/** Tiles are fetched from a remote server during run-time. */
		Remote,
		/** Tiles are fetched from the local file system. */
		Local,
		/** Tiles are fetched from the qrc resources embedded into the executable. */
		Resource
	};

public:
	explicit SimpleMapView(SimpleMapViewBase* parent = nullptr);
	~SimpleMapView() = default;


#ifdef SIMPLE_MAP_VIEW_USE_QML

	/** Registers the QML types on run-time. Call this after creating the app instance. */
	static void registerQmlTypes();

#endif

public slots:
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
	void setTileServer(const QVector<QString>& tileServers);
	/** Gets the tile server source. */
	TileServerSource tileServerSource() const;

	/** Gets the backup server list. */
	const QVector<QString>& backupTileServers() const;
	/** Adds a tile server to the backup server list. */
	void addBackupTileServer(const QString& tileServer);
	/** Adds the tile servers to the backup server list. */
	void addBackupTileServer(const QVector<QString>& tileServers);
	/** Clears the backup tile server list. */
	void clearBackupTileServers();

	/** Checks whether the zoom is locked to the current level. */
	bool isZoomLocked() const;
	/** Sets the zoom lock option. */
	void setLockZoom(bool lock);
	/** Locks the zoom to the current level so it cannot be changed. */
	void lockZoom();
	/** Unlocks the zoom level so it can be changed. */
	void unlockZoom();

	/** Checks whether the geolocation is locked. */
	bool isGeolocationLocked() const;
	/** Sets the geolocation lock option. */
	void setLockGeolocation(bool lock);
	/** Locks the geolocation so it cannot be changed. */
	void lockGeolocation();
	/** Unlocks the geolocation so it can be changed. */
	void unlockGeolocation();

	/** Checks whether changing the zoom level via mouse wheel is disabled. */
	bool isMouseWheelZoomDisabled() const;
	/** Enables/disables changing the zoom level via mouse wheel. */
	void setDisableMouseWheelZoom(bool disable);
	/** Enables changing the zoom level via mouse wheel. */
	void enableMouseWheelZoom();
	/** Disables changing the zoom level via mouse wheel. */
	void disableMouseWheelZoom();

	/** Checks whether moving the map via mouse is disabled. */
	bool isMouseMoveMapDisabled() const;
	/** Enables/disables moving the map via mouse. */
	void setDisableMouseMoveMap(bool disable);
	/** Enables moving the map via mouse. */
	void enableMouseMoveMap();
	/** Disables moving the map via mouse. */
	void disableMouseMoveMap();

	/** Gets the icon used for markers. */
	const QImage& markerIcon() const;
	/** Sets the icon used for markers. */
	void setMarkerIcon(const QImage& icon);
	/** Sets the icon used for markers. */
	void setMarkerIcon(const QString& iconPath);

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
	/** Creates a valid tile server URL string. */
	virtual QString formatTileServerUrlString(QString tileServerUrl, const QPoint& tilePosition, int zoomLevel) const;

	/** Fetches the required tiles from the server and updates the display. */
	void updateMap();
	/** Fetches the tile from the server. */
	void fetchTile(const QPoint& tilePosition);
	/** Fetches the tile from the remote server. */
	void fetchTileFromRemote(const QPoint& tilePosition);
	/** Fetches the tile from the local file system. */
	void fetchTileFromLocal(const QPoint& tilePosition);
	/** Fetches the tile from the qrc resources. */
	void fetchTileFromResource(const QPoint& tilePosition);
	/** Aborts all ongoing requests and drops the replies. */
	void abortReplies();

	/** Gets all visible tiles. */
	std::vector<QString> visibleTiles() const;

	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
#ifndef SIMPLE_MAP_VIEW_USE_QML
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;
#else
	virtual void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
#endif

private:
	void checkTileServers();

#ifndef SIMPLE_MAP_VIEW_USE_QML
	QPainterPath calcPaintClipRegion() const;
	std::array<int, 4> extractBorderRadiiFromStyleSheet() const; // top-left, top-right, bottom-right, bottom-left
	QPen extractBorderPenFromStyleSheet() const;
#endif

private:
	int m_zoomLevel;
	int m_minZoomLevel;
	int m_maxZoomLevel;
	int m_tileCountPerAxis; // pow(2, m_zoomLevel)

	QGeoCoordinate m_center;

	QString m_tileServer;
	TileServerSource m_tileServerSource;
	QNetworkAccessManager m_networkManager;
	int m_tileSize;
	bool m_abortingReplies;

	QVector<QString> m_backupTileServers;
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
	static constexpr unsigned int DOWNLOAD_MAX_CONCURRENT_REQUEST_COUNT = 10;
};

#endif