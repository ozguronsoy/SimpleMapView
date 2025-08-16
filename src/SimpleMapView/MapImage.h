#ifndef MAP_IMAGE_H
#define MAP_IMAGE_H

#include "SimpleMapView/MapRect.h"
#include <QImage>

class MapImage : public MapRect
{
	Q_OBJECT

public:
	explicit MapImage(QObject* parent = nullptr);

	const QImage& image() const;
	void setImage(const QString& imagePath);
	Q_SLOT void setImage(const QImage& img);

	Qt::AspectRatioMode aspectRatioMode() const;
	Q_SLOT void setAspectRatioMode(Qt::AspectRatioMode m);

	virtual void paint(QPainter& painter) const override;

	Q_SIGNAL void imageChanged();

protected:
	virtual QRectF calcPaintRect() const override;

private:
	QImage m_image;
	Qt::AspectRatioMode m_aspectRatioMode;
};

#endif