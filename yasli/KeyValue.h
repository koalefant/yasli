#pragma once
namespace yasli {

class Archive;

class KeyValueInterface : StringInterface
{
public:
	virtual const char* get() const = 0;
	virtual void set(const char* key) = 0;
	virtual bool serializeValue(Archive& ar, const char* name, const char* label) = 0;
	template<class TArchive> void serialize(TArchive& ar)
	{
		ar(*(StringInterface*)this, "", "^");
		serializeValue(ar, "", "^");
	}
};

}

