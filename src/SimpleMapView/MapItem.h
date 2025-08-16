#ifndef MAP_ITEM_H
#define MAP_ITEM_H

#include <QPoint>
#include <QSize>
#include <QGeoCoordinate>
#include <QVariant>
#include <QObject>
#include <QPen>
#include <QPainter>
#include <QMetaType>

class SimpleMapView;

/**
 * @brief Represents a point on a map, which can be stored either in screen coordinates (pixels) or geographic coordinates (degrees).
 */
class MapPoint
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

private:
	QVariant m_val;
};

/**
 * @brief Represents a size on a map, which can be expressed in screen units (pixels) or geographic units (degrees).
 */
class MapSize
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

private:
	QVariant m_val;
};

Q_DECLARE_METATYPE(MapPoint);
Q_DECLARE_METATYPE(MapSize);

/**
 * @brief Base class for items that can be drawn on the map.
 */
class MapItem : public QObject
{
	Q_OBJECT;
	Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penChanged);
	Q_PROPERTY(qreal penWidth READ penWidth WRITE setPenWidth NOTIFY penChanged);
	Q_PROPERTY(Qt::PenStyle penStyle READ penStyle WRITE setPenStyle NOTIFY penChanged);

public:
	explicit MapItem(QObject* parent = nullptr);
	virtual ~MapItem() = default;

	/** Gets the pen used for drawing. */
	const QPen& pen() const;
	/** Sets the pen used for drawing. */
	Q_SLOT void setPen(const QPen& pen);

	/** Gets the pen color. */
	QColor penColor() const;
	/** Sets the pen color. */
	void setPenColor(const QColor& color);

	/** Gets the pen width. */
	qreal penWidth() const;
	/** Sets the pen width. */
	void setPenWidth(qreal width);

	/** Gets the pen style. */
	Qt::PenStyle penStyle() const;
	/** Sets the pen style. */
	void setPenStyle(Qt::PenStyle style);

	/** Draws the item. */
	virtual void paint(QPainter& painter) const = 0;

	/** A signal that's triggered when the map item is changed. */
	Q_SIGNAL void changed();
	/** A signal that's triggered when the pen is changed. */
	Q_SIGNAL void penChanged();

protected:
	/** Gets the pointer to the SimpleMapView instance that the item is drawn on. */
	SimpleMapView* getMapView() const;
	/** Repaints the map. */
	void repaintMap();

private:
	QPen m_pen;
};

#endif