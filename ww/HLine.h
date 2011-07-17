#pragma once

#include "ww/_WidgetWithWindow.h"

class HLineImpl;

namespace ww{

	class WW_API HLine : public _WidgetWithWindow{
	public:
		HLine(int border = 0);
	protected:
		// внутренние функции
		HLineImpl* window() const{ return reinterpret_cast<HLineImpl*>(_window()); }
	};

}

