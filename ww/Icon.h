#pragma once

#include "ww/Color.h"

namespace yasli {
	class Archive;
}

namespace ww {

struct RGBAImage
{
	int width_;
	int height_;
	std::vector<Color> pixels_;

	RGBAImage() : width_(0), height_(0) {}
};


// Icon, stored in XPM format
class Icon
{
public:
	Icon()
	: source_(0)
	, lineCount_(0)
	{
	}
	template<size_t Size>
	Icon(char* (&xpm)[Size])
	{
		set(xpm, Size);
	}

	bool getImage(RGBAImage* out);
	void serialize(yasli::Archive& ar) {}
private:
	void set(const char* const* source, size_t lineCount)
	{
		source_ = source;
		lineCount_ = lineCount;
	}

	const char* const* source_;
	size_t lineCount_;
};

struct IconToggle
{
	bool* variable_;
	bool value_;
	Icon iconTrue_;
	Icon iconFalse_;

	template<size_t Size1, size_t Size2>
	IconToggle(bool& variable, char* (&xpmTrue)[Size1], char* (&xpmFalse)[Size2])
	: iconTrue_(xpmTrue)
	, iconFalse_(xpmFalse)
	, variable_(&variable)
	, value_(variable)
	{
	}

	IconToggle()
	: variable_(0)
	{
	}
	~IconToggle()
	{
		if (variable_)
			*variable_ = value_;
	}

	void serialize(yasli::Archive& ar) {
		//ar(value_, "");
	}
};

}
