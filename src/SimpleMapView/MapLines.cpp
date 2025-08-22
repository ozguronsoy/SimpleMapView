#include "SimpleMapView/MapLines.h"
#include "SimpleMapView.h"

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QSGFlatColorMaterial>

#endif

MapLines::MapLines(QObject* parent)
	: MapItem(parent),
	m_points()
{
	this->setPen(QPen(this->pen().color(), 1));
}

QVector<MapPoint>& MapLines::points()
{
	this->updateMap();
	return m_points;
}

const QVector<MapPoint>& MapLines::points() const
{
	return m_points;
}

void MapLines::setPoints(const QVector<MapPoint>& points)
{
	m_points = points;
}

QVector<QPointF> MapLines::getScreenPoints() const
{
	SimpleMapView* map = this->getMapView();
	QVector<QPointF> screenPoints;

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

void MapLines::render(MapRenderer& renderer) const
{
#ifdef SIMPLE_MAP_VIEW_USE_QML

	const QVector<QPointF> screenPoints = this->getScreenPoints();

	QSGGeometry* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), screenPoints.size());
	geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
	geometry->setLineWidth(this->penWidth());

	QSGGeometry::Point2D* v = geometry->vertexDataAsPoint2D();
	for (size_t i = 0; i < screenPoints.size(); ++i)
	{
		v[i].set(screenPoints[i].x(), screenPoints[i].y());
	}

	QSGFlatColorMaterial* material = new QSGFlatColorMaterial();
	material->setColor(this->penColor());

	QSGGeometryNode* lineNode = new QSGGeometryNode();
	lineNode->setGeometry(geometry);
	lineNode->setFlag(QSGNode::OwnsGeometry);
	lineNode->setMaterial(material);
	lineNode->setFlag(QSGNode::OwnsMaterial);

	renderer.appendChildNode(lineNode);

#else

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		renderer.setPen(this->pen());
		const QVector<QPointF> screenPoints = this->getScreenPoints();
		renderer.drawLines(&screenPoints[0], screenPoints.size() / 2);
	}

#endif
}