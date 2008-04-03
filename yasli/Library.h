#pragma once

#include "yasli/API.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

#include "yasli/LibraryBase.h"
#include "yasli/LibraryReference.h"
#include "yasli/Content.h"
#include "yasli/Errors.h"


// ---------------------------------------------------------------------------

template<class T, class ReferenceType>
class Library : public LibraryBase{
public:
    Library()
    : loaded_(false)
    {
    }

	typedef T ElementType;
    typedef std::vector<T> Elements;

    // virtuals:
    TypeID elementTypeID() const{ return TypeID::get<T>(); }
    TypeID libraryTypeID() const{ return TypeID::get<Library>(); };
    T& objectByIndex(int index){
        return elements_.at(index);
    }
    std::size_t size() const{ return elements_.size(); }

    void serializePreload(Archive& ar)
    {
        ASSERT(ar.isInput());
        ASSERT(!loaded_);
        std::vector<std::string> contents;
        ar(contents, "contents");
        elements_.resize(contents.size());
        for(int i = 0; i < int(contents.size()); ++i){
            elements_[i].setName(contents[i].c_str());
        }
    }
    void serialize(Archive& ar){
        if(!loaded_ && !elements_.empty() && ar.isInput()){
            Elements elements = elements_;
            ar(elements, "elements");
            typename Elements::iterator it;
            for(it = elements.begin(); it != elements.end(); ++it){
                const char* name = it->name();
                int index = findIndexByName(name);
                if(index == -1)
                    elements_.push_back(*it);
                else{
                    std::cout << "Library: Assigning " << index << " (\""
                              << name << "\")" << std::endl;
                    elements_[index] = *it;
                }
            }
            loaded_ = true;
        }
        else{
            std::vector<std::string> contents;
            typename Elements::iterator it;
            for(it = elements_.begin(); it != elements_.end(); ++it){
                const char* name = it->name();
                contents.push_back(name);
            }
            ar(contents, "contents");
            ar(elements_, "elements");
        }
    }
    // ^^^
    T& elementByIndex(int index) 
    {
        ASSERT(index >= 0);
        ASSERT(std::size_t(index) < elements_.size());
        return elements_[index];
    }
    int findIndexByName(const char* name) const{
        typename Elements::const_iterator it;
        int index = 0;
        for(it = elements_.begin(); it != elements_.end(); ++it){
            const LibraryElementBase& element = *it;
            if(strcmp(element.name(), name) == 0)
                return index;
            ++index;
        }
        return -1;
    }

    int addBase(const char* name){
        add(name);
        return elements_.size() - 1;
    }
    ReferenceType add(const char* name){
        T element(name);
        elements_.push_back(element);
        return ReferenceType(elements_.size() - 1);
    }
    void remove(const char* name){
        int elementIndex = findIndexByName(name);
        elements_.erase(elements_.begin() + elementIndex);
    }
	/*
	static Library& the(){
		static Library library;
		return library;
	}
	*/
protected:
    Elements elements_;
    bool loaded_;
};

