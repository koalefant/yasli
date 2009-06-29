#pragma once

#include <vector>
#include <string>
#include "yasli/Assert.h"

namespace yasli{

class Archive;
class StringListStatic : public std::vector<const char*>{
public:
    enum { npos = -1 };
    int find(const char* value) const{
		for(std::size_t i = 0; i < size(); ++i){
            if(std::strcmp((*this)[i], value) == 0)
                return i;
        }
        return npos;
    }
    static StringListStatic EMPTY;
};

class StringListStaticValue{
public:
	StringListStaticValue(const StringListStaticValue& original)
	: stringList_(original.stringList_)
	, index_(original.index_)
	{
	}
    explicit StringListStaticValue(const StringListStatic& stringList = StringListStatic::EMPTY, int value = StringListStatic::npos)
    : stringList_(&stringList)
    , index_(value)
    {
    }
    StringListStaticValue(const StringListStatic& stringList, const char* value)
    : stringList_(&stringList)
    , index_(stringList.find(value))
    {
        ASSERT(index_ != StringListStatic::npos);
    }
    StringListStaticValue& operator=(const char* value){
        index_ = stringList_->find(value);
		return *this;
    }
    StringListStaticValue& operator=(int value){
		ASSERT(value >= 0 && std::size_t(value) < stringList_->size());
        ASSERT(this);
        index_ = value;
		return *this;
    }
    StringListStaticValue& operator=(const StringListStaticValue& rhs){
        stringList_ = rhs.stringList_;
		index_ = rhs.index_;
        return *this;
    }
    const char* c_str() const{
        if(index_ >= 0 && std::size_t(index_) < stringList_->size())
			return (*stringList_)[index_];
		else
			return "";
    }
    int index() const{ return index_; }
    const StringListStatic& stringList() const{ return *stringList_; }
    template<class Archive>
    void serialize(Archive& ar) {
        ar(index_, "index");
    }
protected:
    const StringListStatic* stringList_;
    int index_;
};

class StringList: public std::vector<std::string>{
public:
    StringList() {}
    StringList(const StringList& rhs){
        *this  = rhs;
    }
    StringList(const StringListStatic& rhs){
        int size = int(rhs.size());
        resize(size);
        for(int i = 0; i < int(size); ++i)
            (*this)[i] = rhs[i];
    }
    enum { npos = -1 };
    int find(const char* value) const{
		for(std::size_t i = 0; i < size(); ++i){
            if((*this)[i] == value)
                return i;
        }
        return npos;
    }
    static StringList EMPTY;
};

class StringListValue{
public:
    explicit StringListValue(const StringListStaticValue &value)
	{
        stringList_.insert( stringList_.end(), value.stringList().begin(), value.stringList().end() );
        index_ = value.index();
	}
    StringListValue(const StringListValue &value)
	{
		stringList_ = value.stringList_;
        index_ = value.index_;
	}
    explicit StringListValue(const StringList& stringList = StringList::EMPTY, int value = StringList::npos)
    : stringList_(stringList)
    , index_(value)
    {
    }
    StringListValue(const StringList& stringList, const char* value)
    : stringList_(stringList)
    , index_(stringList.find(value))
    {
        ASSERT(index_ != StringList::npos);
    }
    StringListValue(const StringListStatic& stringList, const char* value)
    : stringList_(stringList)
    , index_(stringList.find(value))
    {
        ASSERT(index_ != StringList::npos);
    }
    StringListValue& operator=(const char* value){
        index_ = stringList_.find(value);
		return *this;
    }
    StringListValue& operator=(int value){
		ASSERT(value >= 0 && std::size_t(value) < stringList_.size());
        ASSERT(this);
        index_ = value;
		return *this;
    }
    const char* c_str() const{
        if(index_ >= 0 && std::size_t(index_) < stringList_.size())
			return stringList_[index_].c_str();
		else
			return "";
    }
    int index() const{ return index_; }
    const StringList& stringList() const{ return stringList_; }
    template<class Archive>
    void serialize(Archive& ar) {
        ar(index_, "index");
		ar(stringList_, "stringList");
    }
protected:
    StringList stringList_;
    int index_;
};

class Archive;

void splitStringList(StringList* result, const char *str, char sep);
void joinStringList(std::string* result, const StringList& stringList, char sep);
void joinStringList(std::string* result, const StringListStatic& stringList, char sep);

}

bool serialize(yasli::Archive& ar, yasli::StringList& value, const char* name, const char* label);
bool serialize(yasli::Archive& ar, yasli::StringListValue& value, const char* name, const char* label);
bool serialize(yasli::Archive& ar, yasli::StringListStaticValue& value, const char* name, const char* label);
