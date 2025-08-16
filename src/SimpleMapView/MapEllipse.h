#ifndef MAP_ELLIPSE_H
#define MAP_ELLIPSE_H

#include "SimpleMapView/MapItem.h"
#include <QSize>
#include <QColor>

class MapEllipse : public MapItem
{
	Q_OBJECT

public:
	explicit MapEllipse(QObject* parent = nullptr);

	Qt::Alignment alignmentFlags() const;
	Q_SLOT void setAlignmentFlags(Qt::Alignment f);

	const MapPoint& position() const;
	void setPosition(const QPointF& position);
	void setPosition(const QGeoCoordinate& position);
	Q_SLOT void setPosition(const MapPoint& position);

	const MapSize& size() const;
	void setSize(const QSizeF& s);
	void setSize(const QGeoCoordinate& s);
	Q_SLOT void setSize(const MapSize& s);

	const QColor& backgroundColor() const;
	Q_SLOT void setBackgroundColor(const QColor& c);

	virtual void paint(QPainter& painter) const override;

	Q_SIGNAL void positionChanged();
	Q_SIGNAL void sizeChanged();
	Q_SIGNAL void backgroundColorChanged();

protected:
	virtual QRectF calcPaintRect() const;
	virtual void applyAlignment(QPointF& p, const QSizeF& s) const;

private:
	Qt::Alignment m_alignmentFlags;
	MapPoint m_position;
	MapSize m_size;
	QColor m_backgroundColor;
};

#endif