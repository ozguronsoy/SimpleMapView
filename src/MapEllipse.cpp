#include "MapEllipse.h"
#include "SimpleMapView.h"
#include <QPainterPath>

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

	this->repaintMap();

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
	this->repaintMap();

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