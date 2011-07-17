#pragma once

#include "ww/_WidgetWithWindow.h"
#include <string>

namespace ww{
	class RadioButtonImpl;
	class RadioButtonGroup;
	class WW_API RadioButton : public _WidgetWithWindow{
	public:
		RadioButton(RadioButton* groupWith = 0, const char* text = "RadioButton", int border = 0);

		const char* text() const { return text_.c_str(); }
		void setText(const char* text);
		void setStatus(bool status);
		bool status() const { return status_; }

		virtual void onChanged();
		sigslot::signal0& signalChanged() { return signalChanged_; }

		void serialize(Archive& ar);
		RadioButtonGroup* group() const;

	protected:
		friend class RadioButtonImpl;
		RadioButtonImpl* window() const;
		// внутренние функции
		sigslot::signal0 signalChanged_;
		std::string text_;
		bool status_;
	};

}

