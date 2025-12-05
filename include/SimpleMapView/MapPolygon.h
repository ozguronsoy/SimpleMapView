#ifndef MAP_POLYGON_H
#define MAP_POLYGON_H

#include "SimpleMapView/MapLines.h"

/**
 * @brief Class for drawing polygons on map.
 */

class MapPolygon : public MapLines
{
	Q_OBJECT;
	Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged);

#ifdef SIMPLE_MAP_VIEW_USE_QML
    QML_ELEMENT;
#endif

public:
	explicit MapPolygon(QObject* parent = nullptr);

	/** Gets the background color. */
	const QColor& backgroundColor() const;
	/** Sets the background color. */
	Q_SLOT void setBackgroundColor(const QColor& c);

	/** A signal that's triggered when the background color is changed. */
	Q_SIGNAL void backgroundColorChanged();

	virtual void render(MapRenderer& renderer) const override;

private:
	QColor m_backgroundColor;
};

#endif
