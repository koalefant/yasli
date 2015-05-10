#include <string>
#include <vector>
using std::vector;

#include "PropertyTree/IMenu.h"
#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/Color.h"
#include "ww/Icon.h"
#include "ww/Decorators.h"
#include "ww/FileSelector.h"
#include "ww/KeyPress.h"

#include "yasli/BitVector.h" 
#include "yasli/decorators/BitFlags.h"

using yasli::Archive;
using yasli::SharedPtr;

#include "Icons/favourites.xpm"
#include "Icons/favouritesDisabled.xpm"
static ww::Icon iconFavourite(favourites_xpm);
static ww::Icon iconFavouriteDisabled(favouritesDisabled_xpm);

enum Flags
{
	FLAG_FIRST = 1 << 0,
	FLAG_SECOND = 1 << 1,
	FLAG_THIRD = 1 << 2,
	FLAG_ALL = FLAG_FIRST | FLAG_SECOND | FLAG_THIRD
};

YASLI_ENUM_BEGIN(Flags, "Flags")
YASLI_ENUM(FLAG_FIRST, "first", "First")
YASLI_ENUM(FLAG_SECOND, "second", "Second")
YASLI_ENUM(FLAG_THIRD, "third", "Third")
YASLI_ENUM(FLAG_ALL, "all", "All")
YASLI_ENUM_END()

struct CustomRows
{
	yasli::BitVector<Flags> bitVector;
	int bitFlags;
	bool notDecorator;
	ww::KeyPress hotkey;

	bool radio1;
	bool radio2;
	bool radio3;

	yasli::string fileSelector;
	bool buttonState;

	CustomRows()
	: bitFlags(FLAG_FIRST | FLAG_THIRD)
	, bitVector(FLAG_FIRST | FLAG_THIRD)
	, notDecorator(true)
	, radio1(false)
	, radio2(true)
	, radio3(false)
	, fileSelector("test_file.txt")
	, buttonState(false)
	{

	}

	void serialize(Archive& ar)
	{
		if (ar.openBlock("wwSpecific", "Rows and Decorators specific to wWidgets")) {
			// old way of serializing bit flags
			ar(bitVector, "bitVector", "Bit Vector");

			ar(ww::NotDecorator(notDecorator), "notDecorator", "Not Decorator");

			if (ar.openBlock("radioGroup", "+Radio Group")) {
				ar(ww::RadioDecorator(radio1), "radio1", "Option 1");
				ar(ww::RadioDecorator(radio2), "radio2", "Option 2");
				ar(ww::RadioDecorator(radio3), "radio3", "Option 3");
				ar.closeBlock();
			}

			static ww::FileSelector::Options options("*.txt", false, ".");
			ar(ww::FileSelector(fileSelector, options), "fileSelector", "File Selector");
			ar(hotkey, "hotkey", "Hotkey");

			ar.closeBlock();
		}


		ar(yasli::HorizontalLine(), "hline", "<");

		if (ar.openBlock("shared", "Shared")) {
			// new way to serialize bit flags, less intrusive
			ar(yasli::BitFlags<Flags>(bitFlags), "bitFlags", "Bit Flags");

			yasli::Button button(buttonState ? "Button: Play" : "Button: Pause");
			ar(button, "button", "<");
			if (button.pressed)
				buttonState = !buttonState;
		}
	}
} customRows;

ww::Widget* createCustomRows()
{
	ww::PropertyTree* propertyTree = new ww::PropertyTree();

	propertyTree->setExpandLevels(1);
	propertyTree->attach(yasli::Serializer(customRows));
	propertyTree->expandAll();

	return propertyTree;
}
