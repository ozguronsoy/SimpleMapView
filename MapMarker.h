#ifndef MAP_MARKER_H
#define MAP_MARKER_H

#include <QObject>
#include <QImage>
#include <QGeoCoordinate>
#include <QString>
#include <QFont>
#include <QColor>

class MapMarker : public QObject
{
	Q_OBJECT

public:
	explicit MapMarker(QObject* parent = nullptr, double latitude = 0.0, double longitude = 0.0, const QString& label = "");

public slots:
	double latitude() const;
	void setLatitude(double latitude);

	double longitude() const;
	void setLongitude(double longitude);

	const QGeoCoordinate& position() const;
	void setPosition(const QGeoCoordinate& position);
	void setPosition(double latitude, double longitude);

	const QString& label() const;
	void setLabel(const QString& label);
	bool hasLabel() const;

	const QFont& labelFont() const;
	void setLabelFont(const QFont& font);

	const QColor& labelColor() const;
	void setLabelColor(const QColor& c);

	const QImage& icon() const;
	void changeIcon(const QString& iconPath);
	void changeIcon(const QImage& icon);

	const QSize& iconSize() const;
	void setIconSize(const QSize& size);
	void setIconSize(double width, double height);

	void replaceIconColor(const QColor& newColor);
	void replaceIconColor(const QColor& oldColor, const QColor& newColor);

signals:
	void positionChanged();
	void labelChanged();
	void iconChanged();

private:
	QGeoCoordinate m_position;
	
	QString m_label;
	QFont m_labelFont;
	QColor m_labelColor;
	
	QImage m_icon;
	QSize m_iconSize;

public:
	static inline QString defaultMarkerIconPath = ":/map_marker.svg";
};

#endif