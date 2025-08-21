#ifndef MAP_RECT_H
#define MAP_RECT_H

#include "SimpleMapView/MapEllipse.h"
#include <array>

/**
 * @brief Class for drawing rectangle on map.
 */
class MapRect : public MapEllipse
{
	Q_OBJECT;

public:
	explicit MapRect(QObject* parent = nullptr);

	/** Gets the border radii. */
	const std::array<qreal, 4>& borderRadii() const;
	/** Sets the border radii. */
	void setBorderRadius(qreal r);
	/** Sets the border radii. */
	void setBorderRadius(const std::array<qreal, 4>& radii);
	/** Sets the border radii. */
	Q_SLOT void setBorderRadius(qreal topLeft, qreal topRight, qreal bottomRight, qreal bottomLeft);

	virtual void paint(QPainter& painter) const override;

	/** A signal that's triggered when the border radius is changed. */
	Q_SIGNAL void borderRadiusChanged();

protected:
	/** Calculates the clip region for applying rounded corners. */
	virtual QPainterPath calcClipRegion() const;

private:
	std::array<qreal, 4> m_borderRadii;  // top-left, top-right, bottom-right, bottom-left
};

#endif