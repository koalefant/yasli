#include "stdafx.h"
#include "ww/ImageStore.h"
#include "ww/Unicode.h"

#include "XMath/Colors.h"
#include "ww/Win32/Window.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>

namespace ww{

ImageStore::ImageStore(int cx, int cy, const char* resourceName, unsigned int maskColor)
{
	InitCommonControls();
	
	imageList_ = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, 0, 16);
	ASSERT(imageList_ != NULL);
	imageListGray_ = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, 0, 16);
	ASSERT(imageListGray_ != NULL);

	size_ = Vect2i(cx, cy);
	if(resourceName)
		addFromResource(resourceName, maskColor);
}

void ImageStore::_createFromBitmap(HBITMAP bitmap, unsigned int color)
{
	BITMAP bitmapStruct;
	ZeroMemory(&bitmapStruct, sizeof(bitmapStruct));

	VERIFY(GetObject(bitmap, sizeof(bitmapStruct), &bitmapStruct));
	ASSERT(bitmapStruct.bmBitsPixel == 32);

	typedef Color4c ColorType;
	int count = bitmapStruct.bmWidth * bitmapStruct.bmHeight;
	ColorType* source = new ColorType[count];
	ColorType* dest = new ColorType[count];
	GetBitmapBits(bitmap, sizeof(ColorType) * count, source);

	int width = bitmapStruct.bmWidth;
	int height = bitmapStruct.bmHeight;
	for(int y = 0; y < height; ++y)
		for(int x = 0; x < width; ++x){
			ColorType c = source[y * width + x];
			if(color != RGB(c.r, c.g, c.b)){
				unsigned char lum = round((c.r * 0.30f + c.g * 0.59f + c.b * 0.11f) * 0.5f + 96);
				dest[y * width + x].set(lum, lum, lum);
			}
			else{
				dest[y * width + x] = c;
			}
		}
	bitmapStruct.bmBits = dest;

	bitmapGray_ = CreateBitmapIndirect(&bitmapStruct);
	ASSERT(bitmapGray_);
	VERIFY(ImageList_AddMasked(imageList_, bitmap, color) != -1);
	VERIFY(ImageList_AddMasked(imageListGray_, bitmapGray_, color) != -1);
	delete[] dest;
	delete[] source;
}

void ImageStore::addFromResource(const char* bitmapID, unsigned int color)
{
	HBITMAP bitmap = LoadBitmap(Win32::_globalInstance(), toWideChar(bitmapID).c_str());
	ASSERT(bitmap);
	_createFromBitmap(bitmap, color);
}

void ImageStore::addFromFile(const char* fileName, unsigned int color)
{
	HBITMAP bitmap = (HBITMAP)LoadImage(Win32::_globalInstance(), toWideChar(fileName).c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	ASSERT(bitmap);
	_createFromBitmap(bitmap, color);
}

ImageStore::~ImageStore()
{
	ImageList_Destroy(imageList_);
}

void ImageStore::_draw(int i, HDC destDC, int x, int y, bool disabled)
{
	VERIFY(ImageList_Draw(disabled ? imageListGray_ : imageList_, i, destDC, x, y, ILD_NORMAL));
}

}
