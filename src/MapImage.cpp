#include "MapImage.h"
#include "SimpleMapView.h"
#include <QPainterPath>

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

	this->repaintMap();

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

	this->repaintMap();

	emit this->changed();
	emit this->imageChanged();
}

void MapImage::paint(QPainter& painter) const
{
	MapRect::paint(painter);

	SimpleMapView* map = this->getMapView();
	if (map != nullptr && !m_image.isNull())
	{
		const QRectF r = this->calcPaintRect();
		const QPainterPath painterPath = this->calcClipRegion();

		painter.setClipPath(painterPath); // apply border radius
		painter.drawImage(
			r.topLeft(),
			m_image.scaled(r.width(), r.height(), m_aspectRatioMode, Qt::SmoothTransformation)
		);
		painter.setClipRect(0, 0, map->width(), map->height()); // reset clip region

		// image is drawn over the border, 
		// hence redraw it
		if (this->pen().widthF() > 0)
		{
			painter.setPen(this->pen());
			painter.drawPath(painterPath);
		}
	}
}

QRectF MapImage::calcPaintRect() const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		// if no fixed or geo size is provided
		// use the image size
		if (!this->geoSize().isValid() && !this->fixedSize().isValid() && !m_image.isNull())
		{
			const QSizeF s = m_image.size();

			QPointF p = map->geoCoordinateToScreenPosition(this->position());
			if (this->alignmentFlags().testFlag(Qt::AlignHCenter))
			{
				p.rx() -= s.width() / 2.0;
			}
			else if (this->alignmentFlags().testFlag(Qt::AlignRight))
			{
				p.rx() -= s.width();
			}

			if (this->alignmentFlags().testFlag(Qt::AlignVCenter))
			{
				p.ry() -= s.height() / 2.0;
			}
			else if (this->alignmentFlags().testFlag(Qt::AlignBottom))
			{
				p.ry() -= s.height();
			}

			return QRectF(p, s);
		}
	}

	return MapRect::calcPaintRect();
}