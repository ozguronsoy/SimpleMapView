#ifndef MAP_ITEM_H
#define MAP_ITEM_H

#include <QObject>
#include <QGeoCoordinate>
#include <QPainter>

class SimpleMapView;

class MapItem : public QObject
{
	Q_OBJECT

public:
	explicit MapItem(QObject* parent = nullptr);
	virtual ~MapItem() = default;

	virtual void paint(QPainter& painter) const = 0;

	Q_SIGNAL void changed();

protected:
	SimpleMapView* getMapView() const;
	void repaintMap();
};

#endif