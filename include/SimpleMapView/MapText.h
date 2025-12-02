#ifndef MAP_TEXT_H
#define MAP_TEXT_H

#include "SimpleMapView/MapRect.h"
#include <QFont>
#include <QMetaType>

Q_DECLARE_METATYPE(QMarginsF);

/**
 * @brief Class for drawing text on map.
 */
class MapText : public MapRect
{
	Q_OBJECT;
	Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged);
	Q_PROPERTY(int textFlags READ textFlags WRITE setTextFlags NOTIFY textFlagsChanged);
	Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged);
	Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged);
	Q_PROPERTY(QMarginsF textPadding READ textPadding WRITE setTextPadding NOTIFY textPaddingChanged);

public:
	explicit MapText(QObject* parent = nullptr);

	/** Gets the text string. */
	const QString& text() const;
	/** Sets the text string. */
	Q_SLOT void setText(const QString& t);

	/** Gets the text flags. */
	int textFlags() const;
	/** Sets the text flags. */
	Q_SLOT void setTextFlags(int flags);

	/** Gets the text color. */
	const QColor& textColor() const;
	/** Sets the text color. */
	Q_SLOT void setTextColor(const QColor& c);

	/** Gets the font. */
	const QFont& font() const;
	/** Sets the font. */
	Q_SLOT void setFont(const QFont& f);

	/** Gets the text padding. */
	const QMarginsF& textPadding() const;
	/** Sets the text padding. */
	void setTextPadding(const QMarginsF& p);
	/** Sets the text padding. */
	Q_SLOT void setTextPadding(qreal left, qreal top, qreal right, qreal bottom);

	virtual void render(MapRenderer& renderer) const override;

	/** A signal that's triggered when the text is changed. */
	Q_SIGNAL void textChanged();
	/** A signal that's triggered when the text flags is changed. */
	Q_SIGNAL void textFlagsChanged();
	/** A signal that's triggered when the text color is changed. */
	Q_SIGNAL void textColorChanged();
	/** A signal that's triggered when the font is changed. */
	Q_SIGNAL void fontChanged();
	/** A signal that's triggered when the text padding is changed. */
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