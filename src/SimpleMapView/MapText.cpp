#include "SimpleMapView/MapText.h"
#include "SimpleMapView.h"
#include <QFontMetrics>

MapText::MapText(QObject* parent)
	: MapRect(parent),
	m_text(),
	m_textFlags(0),
	m_textColor(Qt::white),
	m_font(),
	m_textPadding(5, 5, 5, 5)
{
}

const QString& MapText::text() const
{
	return m_text;
}

void MapText::setText(const QString& t)
{
	m_text = t;

	this->repaintMap();

	emit this->changed();
	emit this->textChanged();
}

int MapText::textFlags() const
{
	return m_textFlags;
}

void MapText::setTextFlags(int flags)
{
	m_textFlags = flags;

	this->repaintMap();

	emit this->changed();
	emit this->textFlagsChanged();
}

const QColor& MapText::textColor() const
{
	return m_textColor;
}

void MapText::setTextColor(const QColor& c)
{
	m_textColor = c;

	this->repaintMap();

	emit this->changed();
	emit this->textColorChanged();
}

const QFont& MapText::font() const
{
	return m_font;
}

void MapText::setFont(const QFont& f)
{
	m_font = f;

	this->repaintMap();

	emit this->changed();
	emit this->fontChanged();
}

const QMarginsF& MapText::textPadding() const
{
	return m_textPadding;
}

void MapText::setTextPadding(const QMarginsF& p)
{
	this->setTextPadding(p.left(), p.top(), p.right(), p.bottom());
}

void MapText::setTextPadding(qreal left, qreal top, qreal right, qreal bottom)
{
	m_textPadding.setLeft(left);
	m_textPadding.setTop(top);
	m_textPadding.setRight(right);
	m_textPadding.setBottom(bottom);

	this->repaintMap();

	emit this->changed();
	emit this->textPaddingChanged();
}

void MapText::paint(QPainter& painter) const
{
	MapRect::paint(painter);

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		const QRectF r = this->calcPaintRect();
		painter.setPen(m_textColor);
		painter.setFont(m_font);
		painter.drawText(r - m_textPadding, m_textFlags, m_text);
	}
}

QRectF MapText::calcPaintRect() const
{
	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		if (!this->size().isValid() && !(m_text.isNull() || m_text.isEmpty()))
		{
			const QFontMetricsF fontMetrics(m_font);
			const QSizeF s = fontMetrics.boundingRect(QRectF(), m_textFlags, m_text).size();
			QPointF p = this->position().screenPoint(map);
			
			this->applyAlignment(p, s);

			return QRectF(p, s) + m_textPadding;
		}
	}

	return MapRect::calcPaintRect() + m_textPadding;
}