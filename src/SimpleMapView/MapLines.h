#ifndef MAP_LINES_H
#define MAP_LINES_H

#include "SimpleMapView/MapItem.h"
#include <vector>

class MapLines : public MapItem
{
	Q_OBJECT

public:
	explicit MapLines(QObject* parent = nullptr);

	std::vector<MapPoint>& points();
	const std::vector<MapPoint>& points() const;

	virtual void paint(QPainter& painter) const override;

protected:
	std::vector<QPointF> getScreenPoints() const;

private:
	std::vector<MapPoint> m_points;
};

#endif