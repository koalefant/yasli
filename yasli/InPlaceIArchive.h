#pragma once

namespace yasli {

class InPlaceIArchive
{
public:
	InPlaceIArchive();
	~InPlaceIArchive() {}

	template<class T>
	const T* load(const char* filename)
	{
		return (const T*)load(sizeof(T), filename);
	}
private:

	const void* load(size_t rootSize, const char* filename);
};

}
