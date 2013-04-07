/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <map>
#include <vector>
#include <QtCore/QRect>
#include "Color.h"

class QPainter;
class QImage;
class QBrush;
class QRect;
class QColor;
class QFont;
struct RGBAImage;
namespace yasli { struct IconXPM; }
struct Color;

struct IconXPMCache
{
	void initialize();
	void finalize();
	void flush();

	~IconXPMCache();

	QImage* getImageForIcon(const yasli::IconXPM& icon);
private:
	struct BitmapCache {
		std::vector<Color> pixels;
		QImage* bitmap;
	};

	static bool parseXPM(RGBAImage* out, const yasli::IconXPM& xpm);
	typedef std::map<const char* const*, BitmapCache> IconToBitmap;
	IconToBitmap iconToImageMap_;
};


void fillRoundRectangle(QPainter& p, const QBrush& brush, const QRect& r, const QColor& borderColor, int radius);
void drawRoundRectangle(QPainter& p, const QRect &_r, unsigned int color, int radius, int width);

QFont* propertyTreeDefaultFont();
QFont* propertyTreeDefaultBoldFont();

enum CheckState {
	CHECK_SET,
	CHECK_NOT_SET,
	CHECK_IN_BETWEEN
};

class QPropertyTree;
struct PropertyDrawContext {
	const QPropertyTree* tree;
	QPainter* painter;
	QRect widgetRect;
	QRect lineRect;
	bool captured;
	bool pressed;

	void drawIcon(const QRect& rect, const yasli::IconXPM& icon) const;
	void drawCheck(const QRect& rect, bool disabled, CheckState checked) const;
	void drawButton(const QRect& rect, const wchar_t* text, bool pressed, bool focused, bool enabled, bool center, const QFont* font) const;
	void drawValueText(bool highlighted, const wchar_t* text) const;
	void drawEntry(const wchar_t* text, bool pathEllipsis, bool grayBackground) const;

	PropertyDrawContext()
	: tree(0)
	, painter(0)
	, captured(false)
	, pressed(false)
	{
	}
};

