#include <unordered_map>
#include <memory>
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

	bool isZoomEnabled() const;
	void setZoomEnabled(bool enabled);

	bool isMapMoveEnabled() const;
	void setMapMoveEnabled(bool enabled);

protected:
	QSize calcRequiredTileCount() const;
	QPointF calcCenterTilePosition() const;
	bool validateTilePosition(const QPoint& tilePosition) const;
	QString getTileKey(const QPoint& tilePosition) const;
	QPoint getTilePosition(const QString& tileKey) const;
	QUrl getTileServerUrl(const QPoint& tilePosition, int zoomLevel) const;
	void updateMap();
	void fetchTile(const QPoint& tilePosition);
	void abortReplies();
	
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	int m_zoomLevel;
	int m_minZoomLevel;
	int m_maxZoomLevel;
	int m_tileCountPerAxis; // pow(2, m_zoomLevel)

	QGeoCoordinate m_center;

	QString m_tileServer;
	QNetworkAccessManager m_networkManager;
	int m_nTilesToFetch;
	int m_fetchedTileCount;
	int m_tileSize;
	bool m_abortingReplies;

	bool m_zoomEnabled;
	bool m_mapMoveEnabled;

	QPoint m_lastMousePosition;

	std::unordered_map<QString, QNetworkReply*> m_replyMap;
	std::unordered_map<QString, std::unique_ptr<QImage>> m_tileMap;
};