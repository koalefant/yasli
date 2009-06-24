#pragma once

template<class T, class Alloc>
bool serialize(Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label)
{
	ContainerSerializationSTLImpl<std::vector<T, Alloc>, T> ser(&container);
	return ar(static_cast<ContainerSerializationInterface&>(ser), name, label);
}

template<class T, class Alloc>
bool serialize(Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label)
{
	ContainerSerializationSTLImpl<std::list<T, Alloc>, T> ser(&container);
	return ar(static_cast<ContainerSerializationInterface&>(ser), name, label);
}
