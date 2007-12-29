#include "StdAfx.h"
#include "PropertyOArchive.h"
#include "PropertyItem.h"
#include "PropertyItemContainer.h"
#include "PropertyItemFactory.h"
#include "PropertyItemBasic.h"

PropertyOArchive::PropertyOArchive(PropertyTreeRoot& root)
: Archive(false, true)
, currentItem_(0)
, lastItem_(0)
, root_(root)
{

}

static PropertyItem* advance(PropertyItem* item)
{
    ASSERT(item);
    const PropertyItem* parent = item->parent();
	ASSERT(parent);

	PropertyItem::const_iterator it = std::find(parent->begin(), parent->end(), item);
	if(it != parent->end() && (++it) != parent->end())
		return *it;
	else{
		ASSERT(!parent->empty());
        return *parent->begin();
	}
}

PropertyItem* PropertyOArchive::add(PropertyItem* newItem, PropertyItem* previousItem)
{
	if(!currentItem_->empty()){
		PropertyItem* nextItem = previousItem ? advance(previousItem) : (PropertyItem*)(*currentItem_->begin());
		if(nextItem->updated() == false && strcmp(nextItem->name(), newItem->name()) == 0){
			ASSERT(nextItem->parent());
			return nextItem->parent()->replace(nextItem, newItem);
		}
		else{
			PropertyItem* oldItem = currentItem_->findByName(newItem->name(), true);
			if(oldItem != 0)
				currentItem_->remove(oldItem);
		}
	}
	return currentItem_->addAfter(newItem, previousItem);
}

bool PropertyOArchive::operator()(const ContainerSerializer& ser, const char* name)
{
    PropertyItemContainer* container = new PropertyItemContainer(name, ser, false);
    currentItem_->add(container);
#ifdef DEBUG
    PropertyItem* oldCurrent = currentItem_;
#endif
    currentItem_ = container;
    int count = int(ser.size());
    for(int i = 0; i < count; ++i)
        ser(*this, i);
    currentItem_ = container->parent();
#ifdef DEBUG
    ASSERT(oldCurrent == currentItem_);
#endif
    return true;
}


bool PropertyOArchive::operator()(const Serializer& ser, const char* name)
{
	ASSERT(!lastItem_ || lastItem_->refCount() >= 1 && lastItem_->refCount() < 10);
    if(currentItem_){
        PropertyItem* item;
        typedef PropertyItemFactory::Constructor Args;
        item = PropertyItemFactory::the()
		.create(ser.type(), Args(name, ser));
        if(!item){
            std::cout << "Now PropertyItem for type " << ser.type().typeInfo()->name() << std::endl;
            item = new PropertyItemStruct(name, ser);
        }
        item = add(item, lastItem_);

		if(item->isLeaf()){
			lastItem_ = item;
            return true;
		}
        currentItem_ = item;
    }
    else{
        currentItem_ = &root_;
    }
	lastItem_ = 0;

	currentItem_->setChildrenUpdated(false);

    ser(*this);

	PropertyItem::iterator it;
	for(it = currentItem_->begin(); it != currentItem_->end(); ){
		PropertyItem* child = *it;
		if(child->updated())
			++it;
		else
			it = currentItem_->erase(it);
	}

	lastItem_ = currentItem_;
    currentItem_ = currentItem_->parent();
    return true;
}

bool PropertyOArchive::operator()(bool& value, const char* name)
{
    lastItem_ = add(new PropertyItemBool(name, value), lastItem_);
    return true;
}

bool PropertyOArchive::operator()(std::string& value, const char* name)
{
    lastItem_ = add(new PropertyItemString(name, value), lastItem_);
    return true;
}

bool PropertyOArchive::operator()(float& value, const char* name)
{
    lastItem_ = add(new PropertyItemFloat(name, value), lastItem_);
    return true;
}

bool PropertyOArchive::operator()(int& value, const char* name)
{
    lastItem_ = add(new PropertyItemInt(name, value), lastItem_);
    return true;
}
