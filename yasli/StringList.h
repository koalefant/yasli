#pragma once
class Archive;
class StringListValue;
class StringListStaticValue;

bool serialize(Archive& ar, StringListValue& value, const char* name);
bool serialize(Archive& ar, StringListStaticValue& value, const char* name);
