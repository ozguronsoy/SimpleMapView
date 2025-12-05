#ifndef MAP_IMAGE_H
#define MAP_IMAGE_H

#include "SimpleMapView/MapRect.h"
#include <QImage>

/**
 * @brief Class for drawing image on map.
 */
class MapImage : public MapRect
{
	Q_OBJECT;
	Q_PROPERTY(QString image WRITE setImage NOTIFY imageChanged);
	Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode WRITE setAspectRatioMode NOTIFY imageChanged);

#ifdef SIMPLE_MAP_VIEW_USE_QML
    QML_ELEMENT;
#endif

public:
	explicit MapImage(QObject* parent = nullptr);

	/** Gets the image. */
	const QImage& image() const;
	/** Sets the image. */
	void setImage(const QString& imagePath);
	/** Sets the image. */
	Q_SLOT void setImage(const QImage& img);

	/** Gets the aspect ratio mode. */
	Qt::AspectRatioMode aspectRatioMode() const;
	/** Sets the aspect ratio mode. */
	Q_SLOT void setAspectRatioMode(Qt::AspectRatioMode m);

	virtual void render(MapRenderer& renderer) const override;

	/** A signal that's triggered when the image is changed. */
	Q_SIGNAL void imageChanged();

protected:
	virtual QRectF calcPaintRect() const override;

private:
	QImage m_image;
	Qt::AspectRatioMode m_aspectRatioMode;
};

#endif
