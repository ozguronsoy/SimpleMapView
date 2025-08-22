#include "SimpleMapView/MapItem.h"
#include "SimpleMapView.h"

MapItem::MapItem(QObject* parent)
	: QObject(parent),
	m_pen(Qt::black, 0.0)
{
}

const QPen& MapItem::pen() const
{
	return m_pen;
}

void MapItem::setPen(const QPen& pen)
{
	m_pen = pen;

	this->updateMap();

	emit this->changed();
	emit this->penChanged();
}

QColor MapItem::penColor() const
{
	return m_pen.color();
}

void MapItem::setPenColor(const QColor& color)
{
	m_pen.setColor(color);
}

qreal MapItem::penWidth() const
{
	return m_pen.widthF();
}

void MapItem::setPenWidth(qreal width)
{
	m_pen.setWidthF(width);
}

Qt::PenStyle MapItem::penStyle() const
{
	return m_pen.style();
}

void MapItem::setPenStyle(Qt::PenStyle style)
{
	m_pen.setStyle(style);
}

SimpleMapView* MapItem::getMapView() const
{
	QObject* parent = this->parent();
	while (parent != nullptr)
	{
		if (parent->inherits("SimpleMapView"))
		{
			return (SimpleMapView*)parent;
		}
		parent = parent->parent();
	}
	return nullptr;
}

void MapItem::updateMap()
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		map->update();
	}
}