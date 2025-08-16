#include "SimpleMapView/MapRect.h"
#include "SimpleMapView.h"
#include <QPainterPath>

MapRect::MapRect(QObject* parent)
	: MapEllipse(parent),
	m_borderRadii({ 0.0, 0.0, 0.0, 0.0 })
{
}

const std::array<qreal, 4>& MapRect::borderRadii() const
{
	return m_borderRadii;
}

void MapRect::setBorderRadius(qreal r)
{
	this->setBorderRadius(r, r, r, r);
}

void MapRect::setBorderRadius(const std::array<qreal, 4>& radii)
{
	this->setBorderRadius(radii[0], radii[1], radii[2], radii[3]);
}

void MapRect::setBorderRadius(qreal topLeft, qreal topRight, qreal bottomRight, qreal bottomLeft)
{
	m_borderRadii[0] = topLeft;
	m_borderRadii[1] = topRight;
	m_borderRadii[2] = bottomRight;
	m_borderRadii[3] = bottomLeft;

	this->repaintMap();

	emit this->changed();
	emit this->borderRadiusChanged();
}

void MapRect::paint(QPainter& painter) const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		QPainterPath painterPath = this->calcClipRegion();
		painter.fillPath(painterPath, this->backgroundColor());

		if (this->pen().widthF() > 0)
		{
			painter.setPen(this->pen());
			painter.drawPath(painterPath);
		}
	}
}

QPainterPath MapRect::calcClipRegion() const
{
	const QRectF r = this->calcPaintRect();

	const qreal topLeftRadius = m_borderRadii[0];
	const qreal topRightRadius = m_borderRadii[1];
	const qreal bottomRightRadius = m_borderRadii[2];
	const qreal bottomLeftRadius = m_borderRadii[3];

	// weird shape is drawn when the if condition returns true
	// don't draw it
	if (r.width() < topLeftRadius || r.height() < topLeftRadius ||
		r.width() < topRightRadius || r.height() < topRightRadius ||
		r.width() < bottomRightRadius || r.height() < bottomRightRadius ||
		r.width() < bottomLeftRadius || r.height() < bottomLeftRadius)
	{
		return QPainterPath();
	}

	QPainterPath painterPath;

	painterPath.moveTo(r.x() + topLeftRadius, r.y());
	painterPath.lineTo(r.x() + r.width() - topRightRadius, r.y());

	painterPath.quadTo(r.x() + r.width(), r.y(), r.x() + r.width(), r.y() + topRightRadius);
	painterPath.lineTo(r.x() + r.width(), r.y() + r.height() - bottomRightRadius);

	painterPath.quadTo(r.x() + r.width(), r.y() + r.height(), r.x() + r.width() - bottomRightRadius, r.y() + r.height());
	painterPath.lineTo(r.x() + bottomLeftRadius, r.y() + r.height());

	painterPath.quadTo(r.x(), r.y() + r.height(), r.x(), r.y() + r.height() - bottomLeftRadius);
	painterPath.lineTo(r.x(), r.y() + topLeftRadius);

	painterPath.quadTo(r.x(), r.y(), r.x() + topLeftRadius, r.y());

	painterPath.closeSubpath();

	return painterPath;
}