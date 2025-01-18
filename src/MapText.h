#ifndef MAP_TEXT_H
#define MAP_TEXT_H

#include "MapRect.h"
#include <QFont>

class MapText : public MapRect
{
	Q_OBJECT

public:
	explicit MapText(QObject* parent = nullptr);

	const QString& text() const;
	Q_SLOT void setText(const QString& t);

	int textFlags() const;
	Q_SLOT void setTextFlags(int flags);

	const QColor& textColor() const;
	Q_SLOT void setTextColor(const QColor& c);

	const QFont& font() const;
	Q_SLOT void setFont(const QFont& f);

	const QMarginsF& textPadding() const;
	void setTextPadding(const QMarginsF& p);
	Q_SLOT void setTextPadding(qreal left, qreal top, qreal right, qreal bottom);

	virtual void paint(QPainter& painter) const override;

	Q_SIGNAL void textChanged();
	Q_SIGNAL void textFlagsChanged();
	Q_SIGNAL void textColorChanged();
	Q_SIGNAL void fontChanged();
	Q_SIGNAL void textPaddingChanged();

protected:
	virtual QRectF calcPaintRect() const override;

private:
	QString m_text;
	int m_textFlags;
	QColor m_textColor;
	QFont m_font;
	QMarginsF m_textPadding;
};

#endif