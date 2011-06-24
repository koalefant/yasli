#include "StdAfx.h"
#include "ww/RadioButtonBox.h"
#include "ww/Serialization.h"
#include "yasli/TypesFactory.h"


namespace ww {
YASLI_CLASS(Widget, RadioButtonBox, "Radio Group");

	RadioButtonBox::RadioButtonBox(const char* text, int border)
	{
		box_ = new ww::VBox(4, 6);
		add(box_);
		radioButtons_.clear();
	}

	RadioButtonBox::~RadioButtonBox()
	{
	}

	int RadioButtonBox::selectedIndex() const
	{
		int i = 0;
		for(i; i < int(radioButtons_.size()); ++i)
			if(radioButtons_[i]->status())
				return i;

		return -1;
	}

	void RadioButtonBox::onChangedSelection()
	{
		signalChangedSelection_.emit();
	}

	void RadioButtonBox::setSelectedIndex(int index)
	{
		radioButtons_[index]->setStatus(true);
	}

	void RadioButtonBox::serialize(Archive& ar)
	{
		//ar.serialize(radioButtons_, "radioButtons", "Кнопки");
		ar.serialize(text_, "text", "Заголовок");
	}

	void RadioButtonBox::addRadioButton(const char * name)
	{
		if(radioButtons_.empty())
			radioButtons_.push_back(new ww::RadioButton(0, name, 0));
		else
			radioButtons_.push_back(new ww::RadioButton(radioButtons_[0], name, 0));
	
		size_t last = radioButtons_.size() - 1;
		box_->add(radioButtons_[last]);
		radioButtons_[last]->signalChanged().connect(this, &RadioButtonBox::onChangedSelection);
	}
}
