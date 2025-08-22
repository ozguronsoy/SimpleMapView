#ifndef MAP_ITEM_H
#define MAP_ITEM_H

#include "SimpleMapView/utils.h"
#include <QObject>
#include <QPen>
#include <QPainter>

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

	/** Renders this item onto the map. */
	virtual void render(MapRenderer& renderer) const = 0;

	/** A signal that's triggered when the map item is changed. */
	Q_SIGNAL void changed();
	/** A signal that's triggered when the pen is changed. */
	Q_SIGNAL void penChanged();

protected:
	/** Gets the pointer to the SimpleMapView instance that the item is drawn on. */
	SimpleMapView* getMapView() const;
	/** Updates the map. */
	void updateMap();

private:
	QPen m_pen;
};

#endif