#pragma once

template<class T, class Alloc>
bool serialize(Archive& ar, std::vector<T, Alloc>& container, const char* name)
{
    return ar(ContainerSerializer(container), name);
}
