#pragma once

#include "Color.h"
#include <vector>
#include "yasli/Assert.h"

#ifdef __GNUC__
using std::size_t;
#endif

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
	, width_(0)
	, height_(0)
	{
	}
    template<size_t Size>
	explicit Icon(char* (&xpm)[Size])
	: width_(0)
	, height_(0)
	{
		set(xpm, Size);
		YASLI_ESCAPE(sscanf_s(xpm[0], "%d %d", &width_, &height_) == 2, return);
	}

	bool getImage(RGBAImage* out) const;
	void serialize(yasli::Archive& ar) {}
	bool operator<(const Icon& rhs) const { return source_ < rhs.source_; }

	int width() const{ return width_; }
	int height() const{ return height_; }
private:
	void set(const char* const* source, size_t lineCount)
	{
		source_ = source;
		lineCount_ = lineCount;
	}

	const char* const* source_;
	size_t lineCount_;
	int width_;
	int height_;
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

	IconToggle(bool& variable, const Icon& iconTrue, const Icon& iconFalse)
	: iconTrue_(iconTrue)
	, iconFalse_(iconFalse)
	, variable_(&variable)
	, value_(variable)
	{
	}

	IconToggle(const IconToggle& orig)
	: variable_(0)
	, value_(orig.value_)
	, iconTrue_(orig.iconTrue_)
	, iconFalse_(orig.iconFalse_)
	{
	}

	IconToggle()
	: variable_(0)
	{
	}
	IconToggle& operator=(const IconToggle& rhs){
		value_ = rhs.value_;
		return *this;
	}
	~IconToggle()
	{
		if (variable_)
			*variable_ = value_;
	}

	void serialize(yasli::Archive& ar);
};