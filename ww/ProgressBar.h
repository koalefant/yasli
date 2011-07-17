#pragma once

#include "ww/_WidgetWithWindow.h"
#include <string>


namespace ww{
	class ProgressBarImpl;
	class WW_API ProgressBar : public _WidgetWithWindow{
	public:
		ProgressBar(int border = 0);
		
		void setPosition(float pos);
		/// [0..1]
		float position() const { return pos_; }
		
		void serialize(Archive& ar);
	protected:
		// внутренние функции
		ProgressBarImpl* window() const{ return reinterpret_cast<ProgressBarImpl*>(_window()); }

		float pos_;
	};

}

