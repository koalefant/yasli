#include "StdAfx.h"
#include "EditorDialog.h"

#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"
#include "yasli/BinaryIArchive.h"
#include "yasli/BinaryOArchive.h"
#include "ww/PropertyTree.h"
#include "ww/Win32/Window.h"


namespace ww{

EditorDialog::EditorDialog(const Serializer& serializer, const char* stateFileName, int flags, ww::Widget* parent)
: ww::Dialog(parent)
{
	init(serializer, stateFileName, flags);
}

EditorDialog::EditorDialog(const Serializer& serializer, const char* stateFileName, int flags, HWND parent)
: ww::Dialog(parent)
{
	init(serializer, stateFileName, flags);
}

void EditorDialog::init(const Serializer& serializer, const char* stateFileName, int flags)
{
	serializer_ = serializer;
	stateFileName_ = stateFileName ? stateFileName : "";
	std::string title = "Property Editor";
	if(!stateFileName_.empty()){
		title += ": ";
		title += stateFileName_;
	}
	setTitle(title.c_str());
	setDefaultSize(450, 500);
	setDefaultPosition(ww::POSITION_CENTER);
	
	setResizeable(true);

	tree_ = new ww::PropertyTree;
	tree_->setImmediateUpdate((flags & ww::IMMEDIATE_UPDATE) != 0);
	tree_->setHideUntranslated((flags & ww::ONLY_TRANSLATED) != 0);
	tree_->setCompact((flags & ww::COMPACT) != 0);
	tree_->attach(serializer);
	if(flags & ww::EXPAND_ALL)
		tree_->expandAll();
	add(tree_, PACK_FILL);

	TextIArchive ia;
	if(stateFileName && ia.load(stateFileName)){
		ia.setFilter(SERIALIZE_STATE);
		ia.serialize(*this, "window", 0);
	}

	if((flags & ww::IMMEDIATE_UPDATE) == 0)
		addButton("Refresh", ww::RESPONSE_RETRY, false);
	else
	{
		tree_->signalChanged().connect(this, &EditorDialog::onTreeChanged);
		originalData_.set(new BinaryOArchive(true));
		serializer(*originalData_);
	}

	addButton("OK", ww::RESPONSE_OK);
	addButton("Cancel", ww::RESPONSE_CANCEL);
}

void EditorDialog::onTreeChanged()
{
	//HWND ownerWnd = Dialog::_ownerWnd();
	HWND ownerWnd = Dialog::parentWnd();
	if (::IsWindow(ownerWnd))
		::RedrawWindow(ownerWnd, 0, 0, RDW_INVALIDATE);
}

void EditorDialog::onResponse(int response)
{
	if(response == ww::RESPONSE_RETRY){
		tree_->apply();
		tree_->revert();
		return;
	}

	if(response == ww::RESPONSE_OK && !tree_->immediateUpdate()){
		tree_->apply();
	}

	if(response == ww::RESPONSE_CANCEL && originalData_)
	{
		BinaryIArchive ia(true);
		if(ia.open(originalData_->buffer(), originalData_->length()))
			serializer_(ia);
	}

	if(!stateFileName_.empty()){
		TextOArchive oa;
		oa.setFilter(SERIALIZE_STATE);
		oa.serialize(*this, "window", 0);
		oa.save(stateFileName_.c_str());
	}
	Dialog::onResponse(response);
}

void EditorDialog::serialize(Archive& ar)
{
	__super::serialize(ar);
    ar.serialize(*tree_, "tree", 0);
}

}
