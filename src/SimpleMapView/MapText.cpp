#include "SimpleMapView/MapText.h"
#include "SimpleMapView.h"
#include <QFontMetrics>

#ifdef SIMPLE_MAP_VIEW_USE_QML

#include <QSGTexture>
#include <QSGTextureMaterial>

#endif

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

	this->updateMap();

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

	this->updateMap();

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

	this->updateMap();

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

	this->updateMap();

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

	this->updateMap();

	emit this->changed();
	emit this->textPaddingChanged();
}

void MapText::render(MapRenderer& renderer) const
{
	MapRect::render(renderer);

#ifdef SIMPLE_MAP_VIEW_USE_QML

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		const QRectF rect = this->calcPaintRect() - m_textPadding;

		QImage textImage(rect.size().toSize(), QImage::Format_RGBA8888);
		textImage.fill(Qt::transparent);

		QPainter painter(&textImage);
		painter.setFont(m_font);
		painter.setPen(m_textColor);
		painter.drawText(QRectF(QPointF(0, 0), rect.size()), m_textFlags, m_text);
		painter.end();

		QSGTexture* texture = map->window()->createTextureFromImage(textImage);
		QSGGeometry* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
		geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);

		QSGGeometry::TexturedPoint2D* v = geometry->vertexDataAsTexturedPoint2D();
		v[0].set(rect.left(), rect.top(), 0, 0);
		v[1].set(rect.right(), rect.top(), 1, 0);
		v[2].set(rect.left(), rect.bottom(), 0, 1);
		v[3].set(rect.right(), rect.bottom(), 1, 1);

		QSGTextureMaterial* material = new QSGTextureMaterial();
		material->setTexture(texture);
		material->setFlag(QSGMaterial::Blending);

		QSGGeometryNode* node = new QSGGeometryNode();
		node->setGeometry(geometry);
		node->setFlag(QSGNode::OwnsGeometry);
		node->setMaterial(material);
		node->setFlag(QSGNode::OwnsMaterial);

		renderer.appendChildNode(node);
	}

#else

	SimpleMapView* map = this->getMapView();
	if (map != nullptr)
	{
		const QRectF r = this->calcPaintRect();
		renderer.setPen(m_textColor);
		renderer.setFont(m_font);
		renderer.drawText(r - m_textPadding, m_textFlags, m_text);
	}

#endif
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