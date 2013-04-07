/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

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
