#include "SimpleMapView/MapPolygon.h"
#include "SimpleMapView.h"

MapPolygon::MapPolygon(QObject* parent)
	: MapLines(parent),
	m_backgroundColor(Qt::transparent)
{
}

const QColor& MapPolygon::backgroundColor() const
{
	return m_backgroundColor;
}

void MapPolygon::setBackgroundColor(const QColor& c)
{
	m_backgroundColor = c;

	this->repaintMap();

	emit this->changed();
	emit this->backgroundColorChanged();
}

void MapPolygon::paint(QPainter& painter) const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		painter.setPen(this->pen());
		painter.setBrush(m_backgroundColor);
		const QVector<QPointF> screenPoints = this->getScreenPoints();
		painter.drawPolygon(&screenPoints[0], screenPoints.size());
		painter.setBrush(Qt::transparent);
	}
}