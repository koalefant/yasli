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
