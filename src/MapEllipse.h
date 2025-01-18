#ifndef MAP_ELLIPSE_H
#define MAP_ELLIPSE_H

#include "MapItem.h"
#include <QSize>
#include <QColor>

class MapEllipse : public MapItem
{
	Q_OBJECT

public:
	explicit MapEllipse(QObject* parent = nullptr);

	Qt::Alignment alignmentFlags() const;
	Q_SLOT void setAlignmentFlags(Qt::Alignment f);

	qreal latitude() const;
	void setLatitude(qreal latitude);

	qreal longitude() const;
	void setLongitude(qreal longitude);

	const QGeoCoordinate& position() const;
	void setPosition(const QGeoCoordinate& position);
	Q_SLOT void setPosition(qreal latitude, qreal longitude);

	qreal geoWidth() const;
	void setGeoWidth(qreal w);

	qreal geoHeight() const;
	void setGeoHeight(qreal h);

	const QGeoCoordinate& geoSize() const;
	void setGeoSize(const QGeoCoordinate& s);
	Q_SLOT void setGeoSize(qreal w, qreal h);

	qreal fixedWidth() const;
	void setFixedWidth(qreal w);

	qreal fixedHeight() const;
	void setFixedHeight(qreal h);

	const QSizeF& fixedSize() const;
	void setFixedSize(const QSizeF& s);
	Q_SLOT void setFixedSize(qreal w, qreal h);

	const QColor& backgroundColor() const;
	Q_SLOT void setBackgroundColor(const QColor& c);

	virtual void paint(QPainter& painter) const override;

	Q_SIGNAL void positionChanged();
	Q_SIGNAL void sizeChanged();
	Q_SIGNAL void backgroundColorChanged();

protected:
	virtual QRectF calcPaintRect() const;

private:
	Qt::Alignment m_alignmentFlags;
	QGeoCoordinate m_position;
	QGeoCoordinate m_geoSize;
	QSizeF m_fixedSize;
	QColor m_backgroundColor;
};

#endif