#pragma once

template<class T, class Alloc>
bool serialize(Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label)
{
	ContainerSerializationSTLImpl<std::vector<T, Alloc>, T> ser(&container);
	return ar(static_cast<const ContainerSerializationInterface&>(ser), name);
}

template<class T, class Alloc>
bool serialize(Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label)
{
	// TODO
	//ContainerSerializationSTLImpl<std::list<T, Alloc>, T> ser(&container);
	//return ar(static_cast<const ContainerSerializationInterface&>(ser), name);
	return false;
}
