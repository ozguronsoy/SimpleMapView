#ifndef MAP_POLYGON_H
#define MAP_POLYGON_H

#include "SimpleMapView/MapLines.h"

class MapPolygon : public MapLines
{
	Q_OBJECT

public:
	explicit MapPolygon(QObject* parent = nullptr);

	const QColor& backgroundColor() const;
	Q_SLOT void setBackgroundColor(const QColor& c);

	Q_SIGNAL void backgroundColorChanged();

	virtual void paint(QPainter& painter) const override;

private:
	QColor m_backgroundColor;

};

#endif