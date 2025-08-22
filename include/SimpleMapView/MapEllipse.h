#ifndef MAP_ELLIPSE_H
#define MAP_ELLIPSE_H

#include "SimpleMapView/MapItem.h"
#include <QSize>
#include <QColor>

/**
 * @brief Class for drawing ellipse on map.
 */
class MapEllipse : public MapItem
{
	Q_OBJECT;
	Q_PROPERTY(Qt::Alignment alignment READ alignmentFlags WRITE setAlignmentFlags NOTIFY positionChanged);
	Q_PROPERTY(MapPoint position READ position WRITE setPosition NOTIFY positionChanged);
	Q_PROPERTY(MapSize size READ size WRITE setSize NOTIFY sizeChanged);
	Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged);

public:
	explicit MapEllipse(QObject* parent = nullptr);

	/** Gets the alignment flags. */
	Qt::Alignment alignmentFlags() const;
	/** Sets the alignment flags. */
	Q_SLOT void setAlignmentFlags(Qt::Alignment f);

	/** Gets the position. */
	const MapPoint& position() const;
	/** Sets the position. */
	void setPosition(const QPointF& position);
	/** Sets the position. */
	void setPosition(const QGeoCoordinate& position);
	/** Sets the position. */
	Q_SLOT void setPosition(const MapPoint& position);

	/** Gets the size. */
	const MapSize& size() const;
	/** Sets the size. */
	void setSize(const QSizeF& s);
	/** Sets the size. */
	void setSize(const QGeoCoordinate& s);
	/** Sets the size. */
	Q_SLOT void setSize(const MapSize& s);

	/** Gets the background color. */
	const QColor& backgroundColor() const;
	/** Sets the background color. */
	Q_SLOT void setBackgroundColor(const QColor& c);

	virtual void render(MapRenderer& renderer) const override;

	/** A signal that's triggered when the position is changed. */
	Q_SIGNAL void positionChanged();
	/** A signal that's triggered when the size is changed. */
	Q_SIGNAL void sizeChanged();
	/** A signal that's triggered when the background color is changed. */
	Q_SIGNAL void backgroundColorChanged();

protected:
	/** Calculates the rectangle the item will be drawn on. */
	virtual QRectF calcPaintRect() const;
	/** Applies Qt::Alignment to the item. */
	virtual void applyAlignment(QPointF& p, const QSizeF& s) const;

private:
	Qt::Alignment m_alignmentFlags;
	MapPoint m_position;
	MapSize m_size;
	QColor m_backgroundColor;
};

#endif