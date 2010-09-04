#pragma once
#include "yasli/Pointers.h"

namespace yasli{ class Archive; }

namespace ww{
class PropertyRow;

struct TreePathLeaf
{
	int index;

	TreePathLeaf(int _index = -1)
		: index(_index)
	{
	}
	bool operator==(const TreePathLeaf& rhs) const{
		return index == rhs.index;
	}
	bool operator!=(const TreePathLeaf& rhs) const{
		return index != rhs.index;
	}
	void serialize(yasli::Archive& ar);
};
typedef std::vector<TreePathLeaf> TreePath;
typedef std::vector<TreePath> TreePathes;

class PropertyTreeOperator
{
public:
    enum Type{
        NONE,
        REPLACE,
        ADD,
        REMOVE
    };

    PropertyTreeOperator();
    ~PropertyTreeOperator();
    PropertyTreeOperator(const TreePath& path, PropertyRow* row);
    void serialize(yasli::Archive& ar);
private:
    Type type_;
    TreePath path_;
    yasli::SharedPtr<PropertyRow> row_;
    int index_;
    friend class PropertyTreeModel;
};



}
