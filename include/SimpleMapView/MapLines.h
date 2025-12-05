#ifndef MAP_LINES_H
#define MAP_LINES_H

#include "SimpleMapView/MapItem.h"
#include <QVector>

/**
 * @brief Class for drawing lines on map.
 */
class MapLines : public MapItem
{
	Q_OBJECT;
	Q_PROPERTY(QVector<MapPoint> points READ points WRITE setPoints);

#ifdef SIMPLE_MAP_VIEW_USE_QML
    QML_ELEMENT;
#endif

public:
	explicit MapLines(QObject* parent = nullptr);

	/** Gets the points. */
	QVector<MapPoint>& points();
	/** Gets the points. */
	const QVector<MapPoint>& points() const;
	/** Sets the points. */
	void setPoints(const QVector<MapPoint>& points);

	virtual void render(MapRenderer& renderer) const override;

protected:
	/** Gets the points as screen points (in px). */
	QVector<QPointF> getScreenPoints() const;

private:
	QVector<MapPoint> m_points;
};

#endif
