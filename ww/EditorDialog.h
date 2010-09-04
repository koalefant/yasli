#pragma once

#include "ww/PropertyEditor.h"
#include "ww/Dialog.h"

namespace yasli{
	class BinaryOArchive;
}


namespace ww{

class PropertyTree;

class EditorDialog : public ww::Dialog{
public:
	EditorDialog(const Serializer& serializer, const char* stateFileName, int flags, ww::Widget*);
	EditorDialog(const Serializer& serializer, const char* stateFileName, int flags, HWND parent);

	void onResponse(int response);
	
	void serialize(Archive& ar);
protected:
	void init(const Serializer& ser, const char* treeStateFileName, int flags);
	void onTreeChanged();

	Serializer serializer_;
	PolyPtr<BinaryOArchive> originalData_;
	yasli::SharedPtr<PropertyTree> tree_;
	std::string stateFileName_;
};
}

