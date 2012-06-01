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
#include "PropertyRowObject.h"

namespace ww{

using std::vector;
using std::string;
using std::map;
using yasli::SharedPtr;

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
class PropertyRowObject;

//////////////////////////////////////////////////////////////////////////

struct ModelObjectReference
{
	SharedPtr<PropertyRowObject> row;
	bool needUpdate;
	bool needApply;

	ModelObjectReference()
	: needApply(false)
	, needUpdate(true)
	{
	}
};
typedef map<void*, ModelObjectReference> ModelObjectReferences;

class PropertyTreeModel : public PolyRefCounter, public has_slots
{
public:
	class LockedUpdate : public RefCounter{
	public:
		LockedUpdate(PropertyTreeModel* model)
		: model_(model)
		{}
		void requestUpdate(const PropertyRows& rows) {			
			for (size_t i = 0; i < rows.size(); ++i) {
				PropertyRow* row = rows[i];
				if (std::find(rows_.begin(), rows_.end(), row) == rows_.end())
					rows_.push_back(row);
			}
		}
		void dismissUpdate(){ rows_.clear(); }
		~LockedUpdate(){
			model_->updateLock_ = 0;
			if(!rows_.empty())
				model_->signalUpdated().emit(rows_);
		}
	protected:
		PropertyTreeModel* model_;
		PropertyRows rows_;
	};
	typedef SharedPtr<LockedUpdate> UpdateLock;

	typedef TreeSelection Selection;

	PropertyTreeModel();
	~PropertyTreeModel();

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
	void requestUpdate(const PropertyRows& rows);
	void dismissUpdate();

	void selectRow(PropertyRow* row, bool selected, bool exclusive = true);
	void deselectAll();

	void push(PropertyRow* row);
	void rowChanged(PropertyRow* row); // be careful: it may recreate/destroy 'row'

	void setUndoEnabled(bool enabled) { undoEnabled_ = enabled; }
	void setFullUndo(bool fullUndo) { fullUndo_ = fullUndo; }
	void setExpandLevels(int levels) { expandLevels_ = levels; }
	int expandLevels() const{ return expandLevels_; }

	typedef signal1<const PropertyRows&> SignalUpdated;
	SignalUpdated& signalUpdated() { return signalUpdated_; };

	void applyOperator(PropertyTreeOperator* op, bool createRedo);
	typedef signal2<PropertyTreeOperator*, bool*> SignalPushUndo;
	SignalPushUndo& signalPushUndo(){ return signalPushUndo_; }

	// for "default archive"
	const StringList& typeStringList(const char* baseTypeName) const;

	bool defaultTypeRegistered(const char* typeName) const;
	void addDefaultType(PropertyRow* propertyRow, const char* typeName);
	PropertyRow* defaultType(const char* baseName, int derivedIndex) const;

	bool defaultTypeRegistered(const char* typeName, const char* derivedName) const;
	void addDefaultType(PropertyRow* propertyRow, const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt);
	PropertyRow* defaultType(const char* typeName) const;

	// for Object rows:
	void registerObjectRow(PropertyRowObject* row);
	void unregisterObjectRow(PropertyRowObject* row);
	void setRootObject(const Object& obj);
	ModelObjectReferences& objectReferences() { return objectReferences_; }

private:
	void pushUndo(const PropertyTreeOperator& op);
	void clearObjectReferences();

	TreePath focusedRow_;
	Selection selection_;
	SignalUpdated signalUpdated_;
	SignalPushUndo signalPushUndo_;

	SharedPtr<PropertyRow> root_;
	Object rootObject_;
	UpdateLock updateLock_;
	ModelObjectReferences objectReferences_;

	typedef map<string, SharedPtr<PropertyRow> > DefaultTypes;
	DefaultTypes defaultTypes_;

	struct DerivedClass{
		string name;
		SharedPtr<PropertyRow> row;
	};
	typedef vector<DerivedClass> DerivedTypes;

	struct BaseClass{
		std::string name;
		StringList strings;
		DerivedTypes types;
	};

	typedef map<string, BaseClass> DefaultTypesPoly;
	DefaultTypesPoly defaultTypesPoly_;
	int expandLevels_;
	bool undoEnabled_;
	bool fullUndo_;

	vector<PropertyTreeOperator> undoOperators_;
	vector<PropertyTreeOperator> redoOperators_;

	friend class TreeImpl;
};

}

bool serialize(yasli::Archive& ar, ww::TreeSelection &selection, const char* name, const char* label);
