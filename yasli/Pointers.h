#pragma once

template<class T>
class SharedPtr;

class Archive;

template<class T>
bool serialize(Archive& ar, SharedPtr<T>& ptr, const char* name);
