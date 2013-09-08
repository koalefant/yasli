/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyDrawContext.h"
#include <memory>
#include "QPropertyTree.h"
#include "yasli/decorators/IconXPM.h"
#include "Unicode.h"
#include <QtGui/QApplication>
#include <QtGui/QStyleOption>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>

#ifndef _MSC_VER
# define _stricmp strcasecmp
#endif

// ---------------------------------------------------------------------------

IconXPMCache::~IconXPMCache()
{
	flush();
}


void IconXPMCache::flush()
{
	IconToBitmap::iterator it;
	for (it = iconToImageMap_.begin(); it != iconToImageMap_.end(); ++it)
		delete it->second.bitmap;
	iconToImageMap_.clear();
}

struct RGBAImage
{
	int width_;
	int height_;
	std::vector<Color> pixels_;

	RGBAImage() : width_(0), height_(0) {}
};

bool IconXPMCache::parseXPM(RGBAImage* out, const yasli::IconXPM& icon) 
{
	if (icon.lineCount < 3) {
		return false;
	}

	// parse values
	std::vector<Color> pixels;
	int width = 0;
	int height = 0;
	int charsPerPixel = 0;
	int colorCount = 0;
	int hotSpotX = -1;
	int hotSpotY = -1;

	int scanResult = sscanf(icon.source[0], "%d %d %d %d %d %d", &width, &height, &colorCount, &charsPerPixel, &hotSpotX, &hotSpotY);
	if (scanResult != 4 && scanResult != 6)
		return false;

	if (charsPerPixel > 4)
		return false;

	if(icon.lineCount != 1 + colorCount + height) {
		YASLI_ASSERT(0 && "Wrong line count");
		return false;
	}

	// parse colors
	std::vector<std::pair<int, Color> > colors;
	colors.resize(colorCount);

	for (int colorIndex = 0; colorIndex < colorCount; ++colorIndex) {
		const char* p = icon.source[colorIndex + 1];
		int code = 0;
		for (int charIndex = 0; charIndex < charsPerPixel; ++charIndex) {
			if (*p == '\0')
				return false;
			code = (code << 8) | *p;
			++p;
		}
		colors[colorIndex].first = code;

		while (*p == '\t' || *p == ' ')
			++p;

		if (*p == '\0')
			return false;

		if (*p != 'c' && *p != 'g')
			return false;
		++p;

		while (*p == '\t' || *p == ' ')
			++p;

		if (*p == '\0')
			return false;

		if (*p == '#') {
			++p;
			if (strlen(p) == 6) {
				int colorCode;
				if(sscanf(p, "%x", &colorCode) != 1)
					return false;
				Color color((colorCode & 0xff0000) >> 16,
							(colorCode & 0xff00) >> 8,
							(colorCode & 0xff),
							255);
				colors[colorIndex].second = color;
			}
		}
		else {
			if(_stricmp(p, "None") == 0)
				colors[colorIndex].second = Color(0, 0, 0, 0);
			else if (_stricmp(p, "Black") == 0)
				colors[colorIndex].second = Color(0, 0, 0, 255)/*GetSysColor(COLOR_BTNTEXT)*/;
			else {
				// unknown color
				colors[colorIndex].second = Color(255, 0, 0, 255);
			}
		}
	}

	// parse pixels
	pixels.resize(width * height);
	int pi = 0;
	for (int y = 0; y < height; ++y) {
		const char* p = icon.source[1 + colorCount + y];
		if (strlen(p) != width * charsPerPixel)
			return false;

		for (int x = 0; x < width; ++x) {
			int code = 0;
			for (int i = 0; i < charsPerPixel; ++i) {
				code = (code << 8) | *p;
				++p;
			}

			for (size_t i = 0; i < size_t(colorCount); ++i)
				if (colors[i].first == code)
					pixels[pi] = colors[i].second;
			++pi;
		}
	}

	out->pixels_.swap(pixels);
	out->width_ = width;
	out->height_ = height;
	return true;
}


QImage* IconXPMCache::getImageForIcon(const yasli::IconXPM& icon)
{
	IconToBitmap::iterator it = iconToImageMap_.find(icon.source);
	if (it != iconToImageMap_.end())
		return it->second.bitmap;

	RGBAImage image;
	if (!parseXPM(&image, icon))
		return 0;

	BitmapCache& cache = iconToImageMap_[icon.source];
	cache.pixels.swap(image.pixels_);
	cache.bitmap = new QImage((unsigned char*)&cache.pixels[0], image.width_, image.height_, QImage::Format_ARGB32);
	return cache.bitmap;
}

// ---------------------------------------------------------------------------

void drawRoundRectangle(QPainter& p, const QRect &_r, unsigned int color, int radius, int width)
{
	QRect r = _r;
	int dia = 2*radius;

	p.setPen(QColor(color));
	p.drawRoundedRect(r, dia, dia);
}

void fillRoundRectangle(QPainter& p, const QBrush& brush, const QRect& _r, const QColor& border, int radius)
{
	bool wasAntialisingSet = p.renderHints().testFlag(QPainter::Antialiasing);
	p.setRenderHints(QPainter::Antialiasing, true);

	p.setBrush(brush);
	QPen pen(QBrush(border), 1.0, Qt::SolidLine);
	p.setPen(pen);
	QRectF adjustedRect = _r;
	adjustedRect.adjust(0.5f, 0.5f, -0.5f, -0.5f);
	p.drawRoundedRect(adjustedRect, radius, radius);

	p.setRenderHints(QPainter::Antialiasing, wasAntialisingSet);
}

// ---------------------------------------------------------------------------

void PropertyDrawContext::drawIcon(const QRect& rect, const yasli::IconXPM& icon) const
{
	QImage* image = tree->_iconCache()->getImageForIcon(icon);
	if (!image)
		return;
	int x = rect.left() + (rect.width() - image->width()) / 2;
	int y = rect.top() + (rect.height() - image->height()) / 2;
	painter->drawImage(x, y, *image);
}

void PropertyDrawContext::drawCheck(const QRect& rect, bool disabled, CheckState checked) const
{
	QStyleOptionButton option;
	if (!disabled)
		option.state |= QStyle::State_Enabled;
	if (checked == CHECK_SET)
		option.state |= QStyle::State_On;
	else if (checked == CHECK_IN_BETWEEN)
		option.state |= QStyle::State_NoChange;
	else
		option.state |= QStyle::State_Off;
	option.rect = QRect(rect.center().x() - 7, rect.center().y() - 6, 13, 13);

	tree->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, 0);
}

void PropertyDrawContext::drawButton(const QRect& rect, const wchar_t* text, bool pressed, bool focused, bool enabled, bool center, const QFont* font) const
{
	QStyleOptionButton option;
	if (enabled)
		option.state |= QStyle::State_Enabled;
	else
		option.state |= QStyle::State_ReadOnly;
	if (pressed) {
		option.state |= QStyle::State_On;
		option.state |= QStyle::State_Sunken;
	}
	else
		option.state |= QStyle::State_Raised;

	if (focused)
		option.state |= QStyle::State_HasFocus;
	option.rect = rect.adjusted(0, 0, -1, -1);
	tree->style()->drawControl(QStyle::CE_PushButton, &option, painter);
	QRect textRect;
	if (enabled)
	{
		QStyleOption arrowOption;
		arrowOption.rect = QRect(rect.right() - 11, rect.top(), 8, rect.height());
		arrowOption.state |= QStyle::State_Enabled;
		tree->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOption, painter, 0);
		textRect = rect.adjusted(0, 0, -8, 0);
	}
	else
		textRect = rect;

	if (pressed)
		textRect = textRect.adjusted(1, 0, 1, 0);
	if (!center)
		textRect.adjust(4, 0, -5, 0);

	QColor textColor = tree->palette().color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText);
	tree->_drawRowValue(*painter, text, font, textRect, textColor, false, center);
}


void PropertyDrawContext::drawValueText(bool highlighted, const wchar_t* text) const
{
	QColor textColor = highlighted ? tree->palette().highlightedText().color() : tree->palette().buttonText().color();
	QRect textRect(widgetRect.left() + 3, widgetRect.top() + 2, widgetRect.width() - 6, widgetRect.height() - 4);
	tree->_drawRowValue(*painter, text, &tree->font(), textRect, textColor, false, false);
}

void PropertyDrawContext::drawEntry(const wchar_t* text, bool pathEllipsis, bool grayBackground, int trailingOffset) const
{
	QRect rt = widgetRect;
	rt.adjust(0, 0, -trailingOffset, -1);

	QStyleOptionFrameV2 option;
	option.state = QStyle::State_Sunken;
	option.lineWidth = tree->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, 0);
	option.midLineWidth = 0;
	option.features = QStyleOptionFrameV2::None;
	if (!grayBackground)
		option.state |= QStyle::State_Enabled;
	if (captured)
	  option.state |= QStyle::State_HasFocus;
	option.rect = rt; // option.rect is the rectangle to be drawn on.
	QRect textRect = tree->style()->subElementRect(QStyle::SE_LineEditContents, &option, 0);
	if (!textRect.isValid())
	{
		textRect = rt;
		textRect.adjust(3, 1, -3, -2);
	}
	// some styles rely on default pens
	painter->setPen(QPen(tree->palette().color(QPalette::WindowText)));
	painter->setBrush(QBrush(tree->palette().color(QPalette::Base)));
	tree->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter, 0);
}


QFont* propertyTreeDefaultFont()
{
	static QFont font;
	return &font;
}

QFont* propertyTreeDefaultBoldFont()
{
	static QFont font;
	font.setBold(true);
	return &font;
}
