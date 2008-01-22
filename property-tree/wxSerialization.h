#pragma once

class Archive;
class wxRect;
class wxWindow;

bool serialize(Archive& ar, wxRect& rect, const char* name);
bool serialize(Archive& ar, wxWindow& window, const char* name);
