#pragma once

#include "ww/Widget.h"
#include "ww/Frame.h"
#include "ww/VBox.h"
#include "ww/RadioButton.h"
#include <string>
#include <vector>


namespace ww{

	class WW_API RadioButtonBox : public ww::Frame 
	{
	public:
		RadioButtonBox(const char* text = "RadioButtonBox", int border = 0);
		~RadioButtonBox();

		void addRadioButton(const char * name);

		void setSelectedIndex(int index);
		int selectedIndex() const;

		virtual void onChangedSelection();
		sigslot::signal0& signalChangedSelection() { return signalChangedSelection_; }

		void serialize(Archive& ar);
		
	protected:
		sigslot::signal0 signalChangedSelection_;

		std::vector<ww::RadioButton*> radioButtons_;
		ww::VBox* box_;
	};
}

