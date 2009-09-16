#pragma once

namespace yasli{ 

class Archive;
struct ButtonDecorator{
	ButtonDecorator(const char* _text = 0)
	: pressed(false)
	, text(_text) {}

	operator bool() const{
		return pressed;
	}
	void serialize(Archive& ar) {}

	bool pressed;
	const char* text;
};

struct HLineDecorator{
	void serialize(Archive& ar) {}
};

struct RadioDecorator{
	RadioDecorator()
	: value_(false)
	, valuePtr_(0)
	{
	}
	RadioDecorator(const RadioDecorator& original)
	: value_(original.value_)
	, valuePtr_(0)
	{
	}
	RadioDecorator(bool& value)
	: value_(value)
	, valuePtr_(&value)
	{}
	~RadioDecorator() { 
		if(valuePtr_)
			*valuePtr_ = value_;
	}

	RadioDecorator& operator=(const RadioDecorator& rhs){
		value_ = rhs.value_;
		return *this;
	}
	RadioDecorator& operator=(bool value){
		value_ = value;
		return *this;
	}
    void serialize(Archive& ar)
    {
        ar(value_, "value", "Value");
    }
	operator bool() const{ return value_; }
	bool value_;
	bool* valuePtr_;
};

struct NotDecorator{
	NotDecorator()
	: value_(false)
	, valuePtr_(0)
	{
	}
	NotDecorator(const NotDecorator& original)
	: value_(original.value_)
	, valuePtr_(0)
	{
	}
	NotDecorator(bool& value)
	: value_(value)
	, valuePtr_(&value)
	{}
	~NotDecorator() { 
		if(valuePtr_)
			*valuePtr_ = value_;
	}

	NotDecorator& operator=(const NotDecorator& rhs){
		value_ = rhs.value_;
		return *this;
	}
	NotDecorator& operator=(bool value){
		value_ = value;
		return *this;
	}
    void serialize(Archive& ar)
    {
        ar(value_, "value", "Value");
    }
	operator bool() const{ return value_; }
	bool value_;
	bool* valuePtr_;
};


template<class T, class PointerType = SharedPtr<T> >
struct OptionalPtr : PointerType{
	void serialize(Archive& ar){
		xassert(ar.isEdit());
		bool optionallyEnabled = false;
		if(ar.isInput()){
			ar.serialize(optionallyEnabled, "optionallyEnabled", "^");
			if(optionallyEnabled){
				if(!get())
					set(new T());
				get()->serialize(ar);
			}
			else
				set(0);
		}
		else{
			optionallyEnabled = get() != 0;
			ar.serialize(optionallyEnabled, "optionallyEnabled", "^");
			if(optionallyEnabled)
				get()->serialize(ar);
		}
	}
};

}

inline bool serialize(yasli::Archive& ar, yasli::NotDecorator& value, const char* name, const char* label){		
    if(ar.isEdit())
        return ar(yasli::Serializer(value), name, label);
    else
        return ar(value.value_, name, label);
}
