/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

template<class T, class Alloc>
bool serialize(yasli::Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label)
{
	yasli::ContainerSerializationSTLImpl<std::vector<T, Alloc>, T> ser(&container);
	return ar(static_cast<yasli::ContainerSerializationInterface&>(ser), name, label);
}

template<class T, class Alloc>
bool serialize(yasli::Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label)
{
	yasli::ContainerSerializationSTLImpl<std::list<T, Alloc>, T> ser(&container);
	return ar(static_cast<yasli::ContainerSerializationInterface&>(ser), name, label);
}
