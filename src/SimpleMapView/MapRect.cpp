#include "SimpleMapView/MapRect.h"
#include "SimpleMapView.h"

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QPainterPath>
#include <QSGFlatColorMaterial>

#else

#include <QPainterPath>

#endif

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

	this->updateMap();

	emit this->changed();
	emit this->borderRadiusChanged();
}

void MapRect::render(MapRenderer& renderer) const
{
#ifdef SIMPLE_MAP_VIEW_USE_QML

	const QRectF rect = this->calcPaintRect();

	QSGGeometryNode* fillNode = new QSGGeometryNode();
	QSGGeometry* fillGeom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 6);
	fillGeom->setDrawingMode(QSGGeometry::DrawTriangles);

	QSGGeometry::Point2D* fv = fillGeom->vertexDataAsPoint2D();
	const float left = rect.left();
	const float top = rect.top();
	const float right = rect.right();
	const float bottom = rect.bottom();

	// fill
	fv[0].set(left, top);
	fv[1].set(right, top);
	fv[2].set(right, bottom);

	fv[3].set(left, top);
	fv[4].set(right, bottom);
	fv[5].set(left, bottom);

	fillNode->setGeometry(fillGeom);
	fillNode->setFlag(QSGNode::OwnsGeometry);

	QSGFlatColorMaterial* fillMaterial = new QSGFlatColorMaterial();
	fillMaterial->setColor(this->backgroundColor());
	fillNode->setMaterial(fillMaterial);
	fillNode->setFlag(QSGNode::OwnsMaterial);

	renderer.appendChildNode(fillNode);

	QSGGeometryNode* borderNode = new QSGGeometryNode();
	QSGGeometry* borderGeom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 24);
	borderGeom->setDrawingMode(QSGGeometry::DrawTriangles);

	// border
	QSGGeometry::Point2D* bv = borderGeom->vertexDataAsPoint2D();

	const float bw = this->penWidth();

	const float l = left;
	const float r = right;
	const float t = top;
	const float b = bottom;

	const float innerL = l + bw;
	const float innerR = r - bw;
	const float innerT = t + bw;
	const float innerB = b - bw;

	// top border
	bv[0].set(l, t);      bv[1].set(r, t);      bv[2].set(r, innerT);
	bv[3].set(l, t);      bv[4].set(r, innerT); bv[5].set(l, innerT);

	// bottom border
	bv[6].set(l, innerB); bv[7].set(r, innerB); bv[8].set(r, b);
	bv[9].set(l, innerB); bv[10].set(r, b);     bv[11].set(l, b);

	// left border
	bv[12].set(l, innerT); bv[13].set(innerL, innerT); bv[14].set(innerL, innerB);
	bv[15].set(l, innerT); bv[16].set(innerL, innerB); bv[17].set(l, innerB);

	// right border
	bv[18].set(innerR, innerT); bv[19].set(r, innerT); bv[20].set(r, innerB);
	bv[21].set(innerR, innerT); bv[22].set(r, innerB); bv[23].set(innerR, innerB);

	borderNode->setGeometry(borderGeom);
	borderNode->setFlag(QSGNode::OwnsGeometry);

	QSGFlatColorMaterial* borderMaterial = new QSGFlatColorMaterial();
	borderMaterial->setColor(this->penColor());
	borderNode->setMaterial(borderMaterial);
	borderNode->setFlag(QSGNode::OwnsMaterial);

	renderer.appendChildNode(borderNode);

#else

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		QPainterPath painterPath = this->calcClipRegion();
		renderer.fillPath(painterPath, this->backgroundColor());

		if (this->pen().widthF() > 0)
		{
			renderer.setPen(this->pen());
			renderer.drawPath(painterPath);
		}
	}

#endif
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