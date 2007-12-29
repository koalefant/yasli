#include "StdAfx.h"
#include <algorithm>
#include "PropertyIArchive.h"
#include "PropertyItem.h"
#include "PropertyItemBasic.h"

PropertyIArchive::PropertyIArchive(const PropertyTreeRoot& root)
: Archive(true, true)
, root_(root)
, currentItem_(0)
, currentChild_(0)
{
}

static const PropertyItem* advance(const PropertyItem* item)
{
    ASSERT(item);
    const PropertyItem* parent = item->parent();
	if(!parent)
		return 0;

	PropertyItem::const_iterator it = std::find(parent->begin(), parent->end(), item);
	if(it != parent->end() && (++it) != parent->end())
		return *it;
	else{
		ASSERT(!parent->empty());
        return *parent->begin();
	}
}

const PropertyItem* PropertyIArchive::findChild(const char* name)
{
	if(currentItem_ == 0){
		currentItem_ = &root_;
		if(!currentItem_->empty())
			currentChild_ = *currentItem_->begin();
		else
			currentChild_ = 0;
		return currentItem_;
	}

	ASSERT(currentChild_);
    const PropertyItem* start = currentChild_;
    const PropertyItem* last = currentChild_;
    do{
        last = currentChild_;
        currentChild_ = advance(currentChild_);
        std::cout << "\t got: " << last->name() << std::endl;
        if(strcmp(last->name(), name) == 0)
            return last;
    }while(currentChild_ != start);
    return 0;
}

bool PropertyIArchive::operator()(const Serializer& ser, const char* name)
{
	const PropertyItem* child = findChild(name);
	if(!child)
		return false;

	currentItem_ = child;
	if(currentItem_->isLeaf()){
		currentItem_->assignTo(ser.pointer(), ser.size());
	}
	else{
		if(!child->empty())
			currentChild_ = *child->begin();
		else
			currentChild_ = 0;

		ser(*this);
	}
    ASSERT(currentItem_);
	currentChild_ = advance(currentItem_);
    currentItem_ = currentItem_->parent();
    return true;
}

bool PropertyIArchive::operator()(bool& value, const char* name)
{
    ASSERT(currentChild_);
    if(const PropertyItem* item = findChild(name)){
		item->assignTo(reinterpret_cast<void*>(&value), sizeof(value));
        return true;
    }
    return false;
}

bool PropertyIArchive::operator()(std::string& value, const char* name)
{
    ASSERT(currentChild_);
    if(const PropertyItem* item = findChild(name)){
		item->assignTo(reinterpret_cast<void*>(&value), sizeof(value));
        return true;
    }
    return false;
}

bool PropertyIArchive::operator()(float& value, const char* name)
{
    ASSERT(currentChild_);
    if(const PropertyItem* item = findChild(name)){
		item->assignTo(reinterpret_cast<void*>(&value), sizeof(value));
        return true;
    }
    return false;
}

bool PropertyIArchive::operator()(int& value, const char* name)
{
    ASSERT(currentChild_);
    if(const PropertyItem* item = findChild(name)){
		item->assignTo(reinterpret_cast<void*>(&value), sizeof(value));
        return true;
    }
    return false;
}
