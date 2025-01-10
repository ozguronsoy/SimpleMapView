#include "MapMarker.h"

MapMarker::MapMarker(const QGeoCoordinate& position, const QString& label)
	: MapMarker(position.latitude(), position.longitude(), label)
{
}

MapMarker::MapMarker(double latitude, double longitude, const QString& label)
	: m_label(label),
	m_labelFont("Arial", 14),
	m_labelColor(Qt::red),
	m_icon(MapMarker::defaultMarkerIconPath),
	m_iconSize(48, 48)
{
	this->setPosition(latitude, longitude);
}

double MapMarker::latitude() const
{
	return m_position.latitude();
}

void MapMarker::setLatitude(double latitude)
{
	this->setPosition(latitude, this->longitude());
}

double MapMarker::longitude() const
{
	return m_position.longitude();
}

void MapMarker::setLongitude(double longitude)
{
	this->setPosition(this->latitude(), longitude);
}

const QGeoCoordinate& MapMarker::position() const
{
	return m_position;
}

void MapMarker::setPosition(const QGeoCoordinate& position)
{
	this->setPosition(position.latitude(), position.longitude());
}

void MapMarker::setPosition(double latitude, double longitude)
{
	m_position.setLatitude(std::min(std::max(latitude, -90.0), 90.0));
	m_position.setLongitude(std::min(std::max(longitude, -180.0), 180.0));

	emit this->positionChanged();
}

const QString& MapMarker::label() const
{
	return m_label;
}

void MapMarker::setLabel(const QString& label)
{
	m_label = label;

	emit this->labelChanged();
}

bool MapMarker::hasLabel() const
{
	return !m_label.isNull() && !m_label.isEmpty();
}

const QFont& MapMarker::labelFont() const
{
	return m_labelFont;
}

void MapMarker::setLabelFont(const QFont& font)
{
	m_labelFont = font;

	emit this->labelChanged();
}

const QColor& MapMarker::labelColor() const
{
	return m_labelColor;
}

void MapMarker::setLabelColor(const QColor& c)
{
	m_labelColor = c;

	emit this->labelChanged();
}

const QImage& MapMarker::icon() const
{
	return m_icon;
}

void MapMarker::changeIcon(const QString& iconPath)
{
	this->changeIcon(QImage(iconPath));
}

void MapMarker::changeIcon(const QImage& icon)
{
	if (!icon.isNull())
	{
		m_icon = icon;

		emit this->iconChanged();
	}
}

const QSizeF& MapMarker::iconSize()
{
	return m_iconSize;
}

void MapMarker::setIconSize(const QSizeF& size)
{
	this->setIconSize(size.width(), size.height());
}

void MapMarker::setIconSize(double width, double height)
{
	this->m_iconSize.setWidth(width);
	this->m_iconSize.setHeight(height);

	emit this->iconChanged();
}

void MapMarker::replaceIconColor(const QColor& newColor)
{
	if (!m_icon.isNull())
	{
		for (int x = 0; x < m_icon.width(); ++x)
		{
			for (int y = 0; y < m_icon.height(); ++y)
			{
				if (m_icon.pixelColor(x, y) != Qt::transparent)
				{
					m_icon.setPixelColor(x, y, newColor);
				}
			}
		}

		emit this->iconChanged();
	}
}

void MapMarker::replaceIconColor(const QColor& oldColor, const QColor& newColor)
{
	if (!m_icon.isNull())
	{
		for (int x = 0; x < m_icon.width(); ++x)
		{
			for (int y = 0; y < m_icon.height(); ++y)
			{
				if (m_icon.pixelColor(x, y) == oldColor)
				{
					m_icon.setPixelColor(x, y, newColor);
				}
			}
		}

		emit this->iconChanged();
	}
}