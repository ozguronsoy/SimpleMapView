#include "MapItem.h"
#include "SimpleMapView.h"

MapItem::MapItem(QObject* parent)
	: QObject(parent)
{
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