#include "SimpleMapView/MapEllipse.h"
#include "SimpleMapView.h"

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QSGFlatColorMaterial>
#include <QtMath>

#else

#include <QPainterPath>

#endif

MapEllipse::MapEllipse(QObject* parent)
	: MapItem(parent),
	m_alignmentFlags(Qt::AlignTop | Qt::AlignLeft),
	m_position(),
	m_size(),
	m_backgroundColor(Qt::black)
{
}

Qt::Alignment MapEllipse::alignmentFlags() const
{
	return m_alignmentFlags;
}

void MapEllipse::setAlignmentFlags(Qt::Alignment f)
{
	m_alignmentFlags = f;

	this->updateMap();

	emit this->changed();
	emit this->positionChanged();
}

const MapPoint& MapEllipse::position() const
{
	return m_position;
}

void MapEllipse::setPosition(const QPointF& position)
{
	this->setPosition(MapPoint(position));
}

void MapEllipse::setPosition(const QGeoCoordinate& position)
{
	this->setPosition(MapPoint(position));
}

void MapEllipse::setPosition(const MapPoint& position)
{
	m_position = position;
	this->updateMap();

	emit this->changed();
	emit this->positionChanged();
}

const MapSize& MapEllipse::size() const
{
	return m_size;
}

void MapEllipse::setSize(const QSizeF& s)
{
	this->setSize(MapSize(s));
}

void MapEllipse::setSize(const QGeoCoordinate& s)
{
	this->setSize(MapSize(s));
}

void MapEllipse::setSize(const MapSize& s)
{
	m_size = s;

	this->updateMap();

	emit this->changed();
	emit this->sizeChanged();
}

const QColor& MapEllipse::backgroundColor() const
{
	return m_backgroundColor;
}

void MapEllipse::setBackgroundColor(const QColor& c)
{
	m_backgroundColor = c;

	this->updateMap();

	emit this->changed();
	emit this->backgroundColorChanged();
}

void MapEllipse::render(MapRenderer& renderer) const
{
#ifdef SIMPLE_MAP_VIEW_USE_QML

	constexpr size_t ellipseSegments = 128;

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		const QRectF rect = this->calcPaintRect();
		const QPointF center = rect.center();
		const float rx = rect.width() / 2.0f;
		const float ry = rect.height() / 2.0f;

		// fill
		QSGGeometryNode* fillNode = new QSGGeometryNode();
		QSGGeometry* fillGeo = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), ellipseSegments * 3);
		fillGeo->setDrawingMode(QSGGeometry::DrawTriangleStrip); // DrawTriangleFan is not supported starting Qt6

		fillNode->setGeometry(fillGeo);
		fillNode->setFlag(QSGNode::OwnsGeometry);

		QSGFlatColorMaterial* fillMat = new QSGFlatColorMaterial();
		fillMat->setColor(m_backgroundColor);
		fillNode->setMaterial(fillMat);
		fillNode->setFlag(QSGNode::OwnsMaterial);

		QSGGeometry::Point2D* fillVertices = fillGeo->vertexDataAsPoint2D();
		for (size_t i = 0; i < ellipseSegments; ++i)
		{
			const float theta0 = (-2.0f * M_PI * i) / ellipseSegments;
			const float theta1 = (-2.0f * M_PI * (i + 1)) / ellipseSegments;

			fillVertices[i * 3 + 0].set(center.x(), center.y());                  
			
			fillVertices[i * 3 + 1].set(
				center.x() + rx * cos(theta0),
				center.y() + ry * sin(theta0)
			);
			
			fillVertices[i * 3 + 2].set(
				center.x() + rx * cos(theta1),
				center.y() + ry * sin(theta1)
			);
		}

		renderer.appendChildNode(fillNode);

		// border
		QSGGeometryNode* borderNode = new QSGGeometryNode();
		QSGGeometry* borderGeo = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), (ellipseSegments + 1) * 2);
		borderGeo->setDrawingMode(QSGGeometry::DrawTriangleStrip);
		borderNode->setGeometry(borderGeo);
		borderNode->setFlag(QSGNode::OwnsGeometry);

		QSGFlatColorMaterial* borderMat = new QSGFlatColorMaterial();
		borderMat->setColor(this->penColor());
		borderNode->setMaterial(borderMat);
		borderNode->setFlag(QSGNode::OwnsMaterial);

		const float halfPenWidth = this->penWidth() / 2.0f;
		QSGGeometry::Point2D* borderVertices = borderGeo->vertexDataAsPoint2D();

		for (size_t i = 0; i <= ellipseSegments; ++i)
		{
			const float theta = (2 * M_PI * i) / ellipseSegments;
			const float cosT = cos(theta);
			const float sinT = sin(theta);

			borderVertices[i * 2].set(
				center.x() + (rx + halfPenWidth) * cosT,
				center.y() + (ry + halfPenWidth) * sinT
			);

			borderVertices[i * 2 + 1].set(
				center.x() + (rx - halfPenWidth) * cosT,
				center.y() + (ry - halfPenWidth) * sinT
			);
		}

		renderer.appendChildNode(borderNode);
	}

#else

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		QPainterPath painterPath;
		painterPath.addEllipse(this->calcPaintRect());

		renderer.fillPath(painterPath, m_backgroundColor);

		if (this->pen().widthF() > 0)
		{
			renderer.setPen(this->pen());
			renderer.drawPath(painterPath);
		}
	}

#endif
}

QRectF MapEllipse::calcPaintRect() const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		QPointF p = m_position.screenPoint(map);
		const QSizeF s = m_size.screenSize(map, m_position);

		this->applyAlignment(p, s);

		return QRectF(p, s);
	}

	return QRectF();
}

void MapEllipse::applyAlignment(QPointF& p, const QSizeF& s) const
{
	if (m_alignmentFlags.testFlag(Qt::AlignHCenter))
	{
		p.rx() -= s.width() / 2.0;
	}
	else if (m_alignmentFlags.testFlag(Qt::AlignRight))
	{
		p.rx() -= s.width();
	}

	if (m_alignmentFlags.testFlag(Qt::AlignVCenter))
	{
		p.ry() -= s.height() / 2.0;
	}
	else if (m_alignmentFlags.testFlag(Qt::AlignBottom))
	{
		p.ry() -= s.height();
	}
}