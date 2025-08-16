#ifndef MAP_RECT_H
#define MAP_RECT_H

#include "SimpleMapView/MapEllipse.h"
#include <array>

class MapRect : public MapEllipse
{
	Q_OBJECT

public:
	explicit MapRect(QObject* parent = nullptr);

	const std::array<qreal, 4>& borderRadii() const;
	void setBorderRadius(qreal r);
	void setBorderRadius(const std::array<qreal, 4>& radii);
	Q_SLOT void setBorderRadius(qreal topLeft, qreal topRight, qreal bottomRight, qreal bottomLeft);

	virtual void paint(QPainter& painter) const override;

	Q_SIGNAL void borderRadiusChanged();

protected:
	virtual QPainterPath calcClipRegion() const;

private:
	std::array<qreal, 4> m_borderRadii;  // top-left, top-right, bottom-right, bottom-left
};

#endif