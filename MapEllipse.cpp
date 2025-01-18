#include "MapEllipse.h"
#include "SimpleMapView.h"
#include <QPainterPath>

MapEllipse::MapEllipse(QObject* parent)
	: MapItem(parent),
	m_alignmentFlags(Qt::AlignTop | Qt::AlignLeft),
	m_position(),
	m_geoSize(),
	m_fixedSize(),
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

	this->repaintMap();
	
	emit this->changed();
	emit this->positionChanged();
}

qreal MapEllipse::latitude() const
{
	return m_position.latitude();
}

void MapEllipse::setLatitude(qreal latitude)
{
	this->setPosition(latitude, this->longitude());
}

qreal MapEllipse::longitude() const
{
	return m_position.longitude();
}

void MapEllipse::setLongitude(qreal longitude)
{
	this->setPosition(this->latitude(), longitude);
}

const QGeoCoordinate& MapEllipse::position() const
{
	return m_position;
}

void MapEllipse::setPosition(const QGeoCoordinate& position)
{
	this->setPosition(position.latitude(), position.longitude());
}

void MapEllipse::setPosition(qreal latitude, qreal longitude)
{
	m_position.setLatitude(std::min(std::max(latitude, -90.0), 90.0));
	m_position.setLongitude(std::min(std::max(longitude, -180.0), 180.0));

	this->repaintMap();

	emit this->changed();
	emit this->positionChanged();
}

qreal MapEllipse::geoWidth() const
{
	return m_geoSize.longitude();
}

void MapEllipse::setGeoWidth(qreal w)
{
	this->setGeoSize(w, this->geoHeight());
}

qreal MapEllipse::geoHeight() const
{
	return m_geoSize.latitude();
}

void MapEllipse::setGeoHeight(qreal h)
{
	this->setGeoSize(this->geoWidth(), h);
}

const QGeoCoordinate& MapEllipse::geoSize() const
{
	return m_geoSize;
}

void MapEllipse::setGeoSize(const QGeoCoordinate& s)
{
	this->setGeoSize(s.longitude(), s.latitude());
}

void MapEllipse::setGeoSize(qreal w, qreal h)
{
	m_geoSize.setLatitude(h);
	m_geoSize.setLongitude(w);

	this->repaintMap();

	emit this->changed();
	emit this->sizeChanged();
}

qreal MapEllipse::fixedWidth() const
{
	return m_fixedSize.width();
}

void MapEllipse::setFixedWidth(qreal w)
{
	this->setFixedSize(w, this->fixedHeight());
}

qreal MapEllipse::fixedHeight() const
{
	return m_fixedSize.height();
}

void MapEllipse::setFixedHeight(qreal h)
{
	this->setFixedSize(this->fixedWidth(), h);
}


const QSizeF& MapEllipse::fixedSize() const
{
	return m_fixedSize;
}

void MapEllipse::setFixedSize(const QSizeF& s)
{
	this->setFixedSize(s.width(), s.height());
}

void MapEllipse::setFixedSize(qreal w, qreal h)
{
	m_fixedSize.setWidth(w);
	m_fixedSize.setHeight(h);

	this->repaintMap();

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

	this->repaintMap();

	emit this->changed();
	emit this->backgroundColorChanged();
}

void MapEllipse::paint(QPainter& painter) const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		QPainterPath painterPath;
		painterPath.addEllipse(this->calcPaintRect());

		painter.fillPath(painterPath, m_backgroundColor);

		if (this->pen().widthF() > 0)
		{
			painter.setPen(this->pen());
			painter.drawPath(painterPath);
		}
	}
}

QRectF MapEllipse::calcPaintRect() const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		QPointF p = map->geoCoordinateToScreenPosition(m_position);

		QSizeF s = m_fixedSize;
		if (s.isNull() || s.isEmpty())
		{
			QPointF end = map->geoCoordinateToScreenPosition(m_position.latitude() + m_geoSize.latitude(), m_position.longitude() + m_geoSize.longitude());
			s = QSizeF(fabs(end.x() - p.x()), fabs(end.y() - p.y()));
		}

		// apply alignment
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

		return QRectF(p, s);
	}

	return QRectF();
}