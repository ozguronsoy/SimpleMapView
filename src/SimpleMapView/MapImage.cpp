#include "SimpleMapView/MapImage.h"
#include "SimpleMapView.h"

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QSGTextureMaterial>

#else

#include <QPainterPath>

#endif

MapImage::MapImage(QObject* parent)
	: MapRect(parent),
	m_image(),
	m_aspectRatioMode(Qt::KeepAspectRatio)
{
	this->setBackgroundColor(Qt::transparent);
}

const QImage& MapImage::image() const
{
	return m_image;
}

void MapImage::setImage(const QString& imagePath)
{
	this->setImage(QImage(imagePath));
}

void MapImage::setImage(const QImage& img)
{
	m_image = img;

	this->updateMap();

	emit this->changed();
	emit this->imageChanged();
}

Qt::AspectRatioMode MapImage::aspectRatioMode() const
{
	return m_aspectRatioMode;
}

void MapImage::setAspectRatioMode(Qt::AspectRatioMode m)
{
	m_aspectRatioMode = m;

	this->updateMap();

	emit this->changed();
	emit this->imageChanged();
}

void MapImage::render(MapRenderer& renderer) const
{
	MapRect::render(renderer);

#ifdef SIMPLE_MAP_VIEW_USE_QML

	SimpleMapView* map = this->getMapView();
	if (map != nullptr && !m_image.isNull())
	{
		const QRectF rect = this->calcPaintRect();

		QSGGeometry* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
		geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);

		QSGGeometry::TexturedPoint2D* v = geometry->vertexDataAsTexturedPoint2D();

		v[0].set(rect.left(), rect.top(), 0, 0);
		v[1].set(rect.right(), rect.top(), 1, 0);
		v[2].set(rect.left(), rect.bottom(), 0, 1);
		v[3].set(rect.right(), rect.bottom(), 1, 1);

		QSGGeometryNode* node = new QSGGeometryNode();
		QSGTextureMaterial* mat = new QSGTextureMaterial();
		mat->setTexture(map->window()->createTextureFromImage(m_image));
		node->setGeometry(geometry);
		node->setMaterial(mat);
		node->setFlag(QSGNode::OwnsGeometry);
		node->setFlag(QSGNode::OwnsMaterial);

		renderer.appendChildNode(node);
	}

#else

	SimpleMapView* map = this->getMapView();
	if (map != nullptr && !m_image.isNull())
	{
		const QRectF r = this->calcPaintRect();
		const QPainterPath painterPath = this->calcClipRegion();

		renderer.setClipPath(painterPath); // apply border radius
		renderer.drawImage(
			r.topLeft(),
			m_image.scaled(r.width(), r.height(), m_aspectRatioMode, Qt::SmoothTransformation)
		);
		renderer.setClipRect(0, 0, map->width(), map->height()); // reset clip region

		// image is drawn over the border, 
		// hence redraw it
		if (this->pen().widthF() > 0)
		{
			renderer.setPen(this->pen());
			renderer.drawPath(painterPath);
		}
	}

#endif

}

QRectF MapImage::calcPaintRect() const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		if (!this->size().isValid() && !m_image.isNull())
		{
			const QSizeF s = m_image.size();
			QPointF p = this->position().screenPoint(map);

			this->applyAlignment(p, s);

			return QRectF(p, s);
		}
	}

	return MapRect::calcPaintRect();
}