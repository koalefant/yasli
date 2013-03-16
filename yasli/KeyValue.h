#pragma once
namespace yasli {

class Archive;

class KeyValueInterface
{
public:
	virtual const char* getKey() = 0;
	virtual void setKey(const char* key) = 0;
	virtual bool serializeValue(Archive& ar) = 0;
};

}

