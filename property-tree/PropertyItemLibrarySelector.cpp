#include "StdAfx.h"
#include "yasli/Serializer.h"
#include "yasli/TypesFactory.h"
#include "yasli/LibraryReference.h"
#include "yasli/Library.h"
#include "PropertyItemFactory.h"
#include "PropertyItemBasic.h"
#include "PropertyTree.h"

#include "res/property_item_library_selector_button.xpm"

class PropertyItemLibrarySelector : public PropertyItemStringList{
public:
	PropertyItemLibrarySelector()
	: PropertyItemStringList("", TypeID())
	{
	}

	PropertyItemLibrarySelector(const char* name, const Serializer& ser)
	: PropertyItemStringList(name, ser.type())
	, selector_(*reinterpret_cast<LibrarySelector*>(ser.pointer()))
	{
		LibraryBase* library = LibraryManager::the().libraryByName(selector_.libraryName());
		const StringList& stringList = library->stringList();
		value_ = StringListValue(stringList, selector_.index());
		ASSERT(ser.size() == sizeof(selector_));

		static wxBitmap bitmap((const char* const *)property_item_library_selector_button_xpm);
		addButton(&bitmap);
	}
	std::string toString() const{
		return value_.c_str();
	}
	void fromString(const char* str){
		value_ = str;

		LibraryBase* library = LibraryManager::the().libraryByName(selector_.libraryName());
		const StringList& stringList = library->stringList();
		selector_.setIndex(stringList.find(str));
	}
	bool onButton(int index, const ViewContext& context){
		if(index == 1){
			context.tree->referenceFollowed(selector_);
			return true;
		}
		else
			return PropertyItemStringList::onButton(index, context);
	}
	void assignTo(void* data, int size) const{
		ASSERT(sizeof(selector_) == size);
		*reinterpret_cast<LibrarySelector*>(data) = selector_;
	}
protected:
	LibrarySelector selector_;
};

REGISTER_PROPERTY_ITEM(LibrarySelector, PropertyItemLibrarySelector)