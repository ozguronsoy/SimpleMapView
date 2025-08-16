#include "SimpleMapView/MapLines.h"
#include "SimpleMapView.h"

MapLines::MapLines(QObject* parent)
	: MapItem(parent),
	m_points()
{
	this->setPen(QPen(this->pen().color(), 1));
}

std::vector<MapPoint>& MapLines::points()
{
	this->repaintMap();
	return m_points;
}

const std::vector<MapPoint>& MapLines::points() const
{
	return m_points;
}

std::vector<QPointF> MapLines::getScreenPoints() const
{
	SimpleMapView* map = this->getMapView();
	std::vector<QPointF> screenPoints;

	if (map != nullptr)
	{
		screenPoints.reserve(m_points.size());

		for (const MapPoint& p : m_points)
		{
			screenPoints.push_back(p.screenPoint(map));
		}
	}

	return screenPoints;
}

void MapLines::paint(QPainter& painter) const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		painter.setPen(this->pen());
		const std::vector<QPointF> screenPoints = this->getScreenPoints();
		painter.drawLines(&screenPoints[0], screenPoints.size() / 2);
	}
}