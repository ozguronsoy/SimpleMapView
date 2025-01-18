#include "MapItem.h"
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

	emit this->changed();
	emit this->penChanged();
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

void MapItem::repaintMap()
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		map->repaint();
	}
}