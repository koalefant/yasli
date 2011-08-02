/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <map>
#include "PropertyRow.h"
#include "PropertyTreeOperator.h"

namespace ww{
using std::vector;

struct TreeSelection : vector<TreePath>
{
	bool operator==(const TreeSelection& rhs){
		if(size() != rhs.size())
			return false;
		for(int i = 0; i < int(size()); ++i)
			if((*this)[i] != rhs[i])
				return false;
	}
};

//////////////////////////////////////////////////////////////////////////

class WW_API PropertyTreeModel : public PolyRefCounter, public has_slots
{
public:
    class LockedUpdate : public RefCounter{
    public:
        LockedUpdate(PropertyTreeModel* model)
        : model_(model)
        , updateRequested_(false)
        {}
        void requestUpdate(){ updateRequested_ = true; }
        void dismissUpdate(){ updateRequested_ = false; }
        ~LockedUpdate(){
            model_->updateLock_ = 0;
            if(updateRequested_)
                model_->signalUpdated().emit();
        }
    protected:
        PropertyTreeModel* model_;
        bool updateRequested_;
    };
    typedef yasli::SharedPtr<LockedUpdate> UpdateLock;

    typedef TreeSelection Selection;

    PropertyTreeModel();

    void clear();
    bool canUndo() const{ return !undoOperators_.empty(); }
    void undo();

    TreePath pathFromRow(PropertyRow* node);
    PropertyRow* rowFromPath(const TreePath& path);
    void setFocusedRow(PropertyRow* row) { focusedRow_ = pathFromRow(row); }
    PropertyRow* focusedRow() { return rowFromPath(focusedRow_); }

    const Selection& selection() const{ return selection_; }
    void setSelection(const Selection& selection);

    void setRoot(PropertyRow* root) { root_ = root; }
    PropertyRow* root() { return root_; }
    const PropertyRow* root() const { return root_; }

    void serialize(Archive& ar, PropertyTree* tree);

    UpdateLock lockUpdate();
    void requestUpdate();
    void dismissUpdate();

    void selectRow(PropertyRow* row, bool selected, bool exclusive = true);
    void deselectAll();

    void push(PropertyRow* row);
    void rowChanged(PropertyRow* row); // be careful: it can destroy 'row'

    void setUndoEnabled(bool enabled) { undoEnabled_ = enabled; }
    void setFullUndo(bool fullUndo) { fullUndo_ = fullUndo; }
    void setExpandLevels(int levels) { expandLevels_ = levels; }
    int expandLevels() const{ return expandLevels_; }

    void onUpdated();
    signal0& signalUpdated() { return signalUpdated_; };

    void applyOperator(PropertyTreeOperator* op, bool createRedo);
    typedef signal2<PropertyTreeOperator*, bool*> SignalPushUndo;
    SignalPushUndo& signalPushUndo(){ return signalPushUndo_; }

    // для defaultArchive
    const StringList& typeStringList(const char* baseTypeName) const;

    bool defaultTypeRegistered(const char* typeName) const;
    void addDefaultType(PropertyRow* propertyRow, const char* typeName);
    PropertyRow* defaultType(const char* baseName, int derivedIndex) const;

    bool defaultTypeRegistered(const char* typeName, const char* derivedName) const;
    void addDefaultType(PropertyRow* propertyRow, const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt);
    PropertyRow* defaultType(const char* typeName) const;

private:
    void pushUndo(const PropertyTreeOperator& op);

    TreePath focusedRow_;
    Selection selection_;
    signal0 signalUpdated_;
    SignalPushUndo signalPushUndo_;

    yasli::SharedPtr<PropertyRow> root_;
    UpdateLock updateLock_;

    typedef std::map<std::string, SharedPtr<PropertyRow> > DefaultTypes;
    DefaultTypes defaultTypes_;

    struct DerivedClass{
        std::string name;
        SharedPtr<PropertyRow> row;
    };
    typedef std::vector<DerivedClass> DerivedTypes;

    struct BaseClass{
        std::string name;
        StringList strings;
        DerivedTypes types;
    };

    typedef std::map<std::string, BaseClass> DefaultTypesPoly;
    DefaultTypesPoly defaultTypesPoly_;
    int expandLevels_;
    bool undoEnabled_;
    bool fullUndo_;

    std::vector<PropertyTreeOperator> undoOperators_;
    std::vector<PropertyTreeOperator> redoOperators_;

    friend class TreeImpl;
};

}

bool serialize(yasli::Archive& ar, ww::TreeSelection &selection, const char* name, const char* label);
