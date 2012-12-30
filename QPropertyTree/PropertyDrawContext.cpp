/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyDrawContext.h"
#include <memory>
#include "QPropertyTree.h"
#include "Icon.h"
#include <QtGui/QApplication>
#include <QtGui/QStyleOption>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>

// ---------------------------------------------------------------------------

static DrawingCache drawingCache;

DrawingCache* DrawingCache::get()
{
	return &drawingCache;
}

DrawingCache::~DrawingCache()
{
}

void DrawingCache::initialize()
{
	//Gdiplus::GdiplusStartupInput startup;
	//startup.DebugEventCallback = 0;
	//startup.GdiplusVersion = 1;
	//startup.SuppressBackgroundThread = FALSE;

	//if(users_ == 0){
	//	uxTheme.loadLibrary();
	//	theme_ = uxTheme.OpenThemeData(Win32::getDefaultWindowHandle(), L"Button");

	//	YASLI_ESCAPE(GdiplusStartup(&token_, &startup, 0) == Ok, return);

	//	NONCLIENTMETRICS nonClientMetrics;
	//	nonClientMetrics.cbSize = sizeof(nonClientMetrics);
	//	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), (PVOID)&nonClientMetrics, 0);
	//	defaultFont.reset(new Gdiplus::Font(GetDC(GetDesktopWindow()), &nonClientMetrics.lfMessageFont));

	//	nonClientMetrics.lfMessageFont.lfWeight = FW_BOLD;
	//	defaultBoldFont.reset(new Gdiplus::Font(GetDC(GetDesktopWindow()), &nonClientMetrics.lfMessageFont));
	//}

	++users_;
}

void DrawingCache::flush()
{
	IconToBitmap::iterator it;
	for (it = iconToBitmapMap_.begin(); it != iconToBitmapMap_.end(); ++it)
		delete it->second.bitmap;
	iconToBitmapMap_.clear();
}

void DrawingCache::finalize()
{
	--users_;
	if(users_ == 0)
	{
		flush();
	}
}

QBitmap* DrawingCache::getBitmapForIcon(const Icon& icon)
{
	return 0;
	// IconToBitmap::iterator it = iconToBitmapMap_.find(icon);
	// if (it != iconToBitmapMap_.end())
	// 	return it->second.bitmap;

	// RGBAImage image;
	// YASLI_ESCAPE(icon.getImage(&image), return 0);

	// BitmapCache& cache = iconToBitmapMap_[icon];
	// cache.pixels.swap(image.pixels_);
	// cache.bitmap = new Gdiplus::Bitmap(image.width_, image.height_, image.width_ * 4, PixelFormat32bppARGB, (BYTE*)&cache.pixels[0]);
	// return cache.bitmap;
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

void PropertyDrawContext::drawIcon(const QRect& rect, const Icon& icon) const
{
	// Gdiplus::Bitmap* bitmap = DrawingCache::get()->getBitmapForIcon(icon);
	// YASLI_ESCAPE(bitmap != 0, return);
	// int x = rect.left() + (rect.width() - icon.width()) / 2;
	// int y = rect.top() + (rect.height() - icon.height()) / 2;
	// graphics->DrawImage(bitmap, x, y);
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
	option.rect = rect;
	
	
	//QRect rect = tree->style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, 0);
	tree->style()->drawControl(QStyle::CE_CheckBox, &option, painter);
}

void PropertyDrawContext::drawButton(const QRect& rect, const wchar_t* text, bool pressed, bool focused) const
{
		QStyleOptionButton option;
		option.text = QString::fromUtf16((ushort*)text);
		option.state |= QStyle::State_Enabled;
		if (pressed)
			option.state |= QStyle::State_On;
		if (focused)
			option.state |= QStyle::State_HasFocus;
		option.rect = rect;
		tree->style()->drawControl(QStyle::CE_PushButton, &option, painter);

}


void PropertyDrawContext::drawValueText(bool highlighted, const wchar_t* text) const
{
	QColor textColor = highlighted ? tree->palette().highlightedText().color() : tree->palette().buttonText().color();
	QRect textRect(widgetRect.left() + 3, widgetRect.top() + 2, widgetRect.right() - 3, widgetRect.bottom() - 2);
	tree->_drawRowValue(*painter, text, 0, textRect, textColor, false, false);
}

void PropertyDrawContext::drawEntry(const wchar_t* text, bool pathEllipsis, bool grayBackground) const
{
	{
		QRect rt = widgetRect;
		rt.adjust(0, 0, 0, -1);

		QStyleOption option;
		option.state = QStyle::State_Sunken | QStyle::State_Editing;
		if (!grayBackground)
			option.state |= QStyle::State_Enabled;
		option.rect = rt; // option.rect is the rectangle to be drawn on.
		QRect textRect = tree->style()->subElementRect(QStyle::SE_LineEditContents, &option, 0);
		if (!textRect.isValid())
		{
			textRect = rt;
			textRect.adjust(3, 1, -3, -2);
		}
		painter->fillRect(rt, tree->palette().base());
		tree->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter, tree);
		tree->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, painter, tree);
		painter->setPen(QPen(tree->palette().color(QPalette::WindowText)));
		painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString::fromUtf16((ushort*)text), 0);
	}
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
