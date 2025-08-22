#include "SimpleMapView/MapPolygon.h"
#include "SimpleMapView.h"

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QSGFlatColorMaterial>

#endif

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

	this->updateMap();

	emit this->changed();
	emit this->backgroundColorChanged();
}

void MapPolygon::render(MapRenderer& renderer) const
{
#ifdef SIMPLE_MAP_VIEW_USE_QML

	const QVector<QPointF> screenPoints = this->getScreenPoints();

	// fill
	QSGGeometryNode* fillNode = new QSGGeometryNode();
	QSGGeometry* fillGeo = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), (screenPoints.size() - 2) * 3);
	fillGeo->setDrawingMode(QSGGeometry::DrawTriangles);
	fillNode->setGeometry(fillGeo);
	fillNode->setFlag(QSGNode::OwnsGeometry);

	QSGFlatColorMaterial* fillMat = new QSGFlatColorMaterial();
	fillMat->setColor(m_backgroundColor);
	fillNode->setMaterial(fillMat);
	fillNode->setFlag(QSGNode::OwnsMaterial);

	QSGGeometry::Point2D* v = fillGeo->vertexDataAsPoint2D();
	for (size_t i = 1; i < screenPoints.size() - 1; ++i)
	{
		v[(i - 1) * 3 + 0].set(screenPoints[0].x(), screenPoints[0].y());
		v[(i - 1) * 3 + 1].set(screenPoints[i].x(), screenPoints[i].y());
		v[(i - 1) * 3 + 2].set(screenPoints[i + 1].x(), screenPoints[i + 1].y());
	}

	renderer.appendChildNode(fillNode);

	// border
	QSGGeometryNode* borderNode = new QSGGeometryNode();
	QSGGeometry* borderGeo = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), screenPoints.size() + 1);
	borderGeo->setDrawingMode(QSGGeometry::DrawLineStrip);
	borderGeo->setLineWidth(this->penWidth());
	borderNode->setGeometry(borderGeo);
	borderNode->setFlag(QSGNode::OwnsGeometry);

	QSGFlatColorMaterial* borderMat = new QSGFlatColorMaterial();
	borderMat->setColor(this->penColor());
	borderNode->setMaterial(borderMat);
	borderNode->setFlag(QSGNode::OwnsMaterial);

	QSGGeometry::Point2D* bv = borderGeo->vertexDataAsPoint2D();
	for (size_t i = 0; i < screenPoints.size(); ++i)
	{
		bv[i].set(screenPoints[i].x(), screenPoints[i].y());
	}
	bv[screenPoints.size()] = bv[0];

	renderer.appendChildNode(borderNode);

#else

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		renderer.setPen(this->pen());
		renderer.setBrush(m_backgroundColor);
		const QVector<QPointF> screenPoints = this->getScreenPoints();
		renderer.drawPolygon(&screenPoints[0], screenPoints.size());
		renderer.setBrush(Qt::transparent);
	}

#endif
}