#pragma once
#include "yasli/Archive.h"
#include "LibrarySelector.h"

template<class ElementType, class LibraryType>
int LibraryReference<ElementType, LibraryType>::findIndexByName(const char* elementName) const
{
	return LibraryType::the().findIndexByName(elementName);
}

template<class ElementType, class LibraryType>
const char* LibraryReference<ElementType, LibraryType>::libraryName() const
{
	return LibraryType::the().name();
}

template<class ElementType, class LibraryType>
LibraryElementBase* LibraryReference<ElementType, LibraryType>::elementByIndex(int index) const
{
	return &LibraryType::the().elementByIndex(index);
}

template<class ElementType, class LibraryType>
void LibraryReference<ElementType, LibraryType>::serialize(Archive& ar)
{
	if(ar.isEdit()){
		const char* name = libraryName();
		LibrarySelector selector(this, name);
		ar(selector, "");
	}
	else{
		ElementType* element = get();
		std::string value;
		if(ar.isOutput() && element)
			value = ((LibraryElementBase*)(element))->name();

		ar(value, "");

		if(ar.isInput())
			index_ = findIndexByName(value.c_str());
	}
}

template<class ElementType, class LibraryType>
bool LibraryReference<ElementType, LibraryType>::instantiate()
{
	LibraryReference ref;
	ref.elementByIndex(0);
	ref.libraryName();
	ref.findIndexByName("");
	ref.serialize(*(Archive*)(0));
	return true;
}
