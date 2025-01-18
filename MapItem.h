#ifndef MAP_ITEM_H
#define MAP_ITEM_H

#include <QObject>
#include <QGeoCoordinate>
#include <QPen>
#include <QPainter>

class SimpleMapView;

class MapItem : public QObject
{
	Q_OBJECT

public:
	explicit MapItem(QObject* parent = nullptr);
	virtual ~MapItem() = default;

	const QPen& pen() const;
	Q_SLOT void setPen(const QPen& pen);

	virtual void paint(QPainter& painter) const = 0;

	Q_SIGNAL void changed();
	Q_SIGNAL void penChanged();

protected:
	SimpleMapView* getMapView() const;
	void repaintMap();

private:
	QPen m_pen;
};

#endif