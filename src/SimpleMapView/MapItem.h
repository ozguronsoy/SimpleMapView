#ifndef MAP_ITEM_H
#define MAP_ITEM_H

#include <QPoint>
#include <QSize>
#include <QGeoCoordinate>
#include <QVariant>
#include <QObject>
#include <QPen>
#include <QPainter>

class SimpleMapView;

class MapPoint : protected QVariant
{
public:
	MapPoint();
	MapPoint(const QPointF& screenPoint);
	MapPoint(const QGeoCoordinate& geoPoint);

	MapPoint& operator=(const QPointF& screenPoint);
	MapPoint& operator=(const QGeoCoordinate& geoPoint);

	bool isValid() const;
	QPointF screenPoint(const SimpleMapView* map) const;
	QGeoCoordinate geoPoint(const SimpleMapView* map) const;
};

class MapSize : protected QVariant
{
public:
	MapSize();
	MapSize(const QSizeF& screenSize);
	MapSize(const QGeoCoordinate& geoSize);

	MapSize& operator=(const QPointF& screenSize);
	MapSize& operator=(const QGeoCoordinate& geoSize);

	bool isValid() const;
	QSizeF screenSize(const SimpleMapView* map, const MapPoint& topLeft) const;
	QGeoCoordinate geoSize(const SimpleMapView* map, const MapPoint& topLeft) const;
};

class MapItem : public QObject
{
	Q_OBJECT

public:
	explicit MapItem(QObject* parent = nullptr);
	virtual ~MapItem() = default;

	const QPen& pen() const;
	Q_SLOT void setPen(const QPen& pen);

	virtual void paint(QPainter& painter) const = 0;

	Q_SIGNAL void changed();
	Q_SIGNAL void penChanged();

protected:
	SimpleMapView* getMapView() const;
	void repaintMap();

private:
	QPen m_pen;
};

#endif