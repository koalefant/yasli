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
#include "PropertyRowObject.h"
#include "PropertyTreeOperator.h"
#include "yasli/Pointers.h"

using std::vector;
using std::map;

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

struct PropertyDefaultTypeValue
{
	yasli::TypeID type;
	yasli::SharedPtr<PropertyRow> root;
	yasli::ClassFactoryBase* factory;
	int factoryIndex;
	yasli::string label;

	PropertyDefaultTypeValue()
	: factoryIndex(-1)
	, factory(0)
	{
	}
};

// ---------------------------------------------------------------------------
struct ModelObjectReference
{
	yasli::SharedPtr<PropertyRowObject> row;
	bool needUpdate;
	bool needApply;

	ModelObjectReference()
	: needApply(false)
	, needUpdate(true)
	{
	}
};
typedef map<void*, ModelObjectReference> ModelObjectReferences;

class PropertyTreeModel : public QObject
{
	Q_OBJECT
public:
	class LockedUpdate : public yasli::RefCounter{
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
				model_->signalUpdated(rows_);
		}
	protected:
		PropertyTreeModel* model_;
		PropertyRows rows_;
	};
	typedef yasli::SharedPtr<LockedUpdate> UpdateLock;

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

	void serialize(yasli::Archive& ar, QPropertyTree* tree);

	UpdateLock lockUpdate();
	void requestUpdate(const PropertyRows& rows);
	void dismissUpdate();

	void selectRow(PropertyRow* row, bool selected, bool exclusive = true);
	void deselectAll();

	void push(PropertyRow* row);
	void rowChanged(PropertyRow* row); // be careful: it can destroy 'row'

	void setUndoEnabled(bool enabled) { undoEnabled_ = enabled; }
	void setFullUndo(bool fullUndo) { fullUndo_ = fullUndo; }
	void setExpandLevels(int levels) { expandLevels_ = levels; }
	int expandLevels() const{ return expandLevels_; }

	void onUpdated(const PropertyRows& rows);

	void applyOperator(PropertyTreeOperator* op, bool createRedo);

	// for defaultArchive
	const yasli::StringList& typeStringList(const yasli::TypeID& baseType) const;

	bool defaultTypeRegistered(const char* typeName) const;
	void addDefaultType(PropertyRow* propertyRow, const char* typeName);
	PropertyRow* defaultType(const char* typeName) const;

	bool defaultTypeRegistered(const yasli::TypeID& baseType, const yasli::TypeID& derivedType) const;
	void addDefaultType(const yasli::TypeID& baseType, const PropertyDefaultTypeValue& value);
	const PropertyDefaultTypeValue* defaultType(const yasli::TypeID& baseType, int index) const;
	
	// for Object rows:
	void registerObjectRow(PropertyRowObject* row);
	void unregisterObjectRow(PropertyRowObject* row);
	void setRootObject(const yasli::Object& obj);
	ModelObjectReferences& objectReferences() { return objectReferences_; }
signals:
	void signalUpdated(const PropertyRows& rows);
	void signalPushUndo(PropertyTreeOperator* op, bool* result);
private:
	void pushUndo(const PropertyTreeOperator& op);
	void clearObjectReferences();

	TreePath focusedRow_;
	Selection selection_;

	yasli::SharedPtr<PropertyRow> root_;
	yasli::Object rootObject_;
	UpdateLock updateLock_;
	ModelObjectReferences objectReferences_;

	typedef std::map<yasli::string, yasli::SharedPtr<PropertyRow> > DefaultTypes;
	DefaultTypes defaultTypes_;


	typedef vector<PropertyDefaultTypeValue> DerivedTypes;
	struct BaseClass{
		yasli::TypeID type;
		yasli::string name;
		yasli::StringList strings;
		DerivedTypes types;
	};
	typedef map<yasli::TypeID, BaseClass> DefaultTypesPoly;
	DefaultTypesPoly defaultTypesPoly_;

	int expandLevels_;
	bool undoEnabled_;
	bool fullUndo_;

	std::vector<PropertyTreeOperator> undoOperators_;
	std::vector<PropertyTreeOperator> redoOperators_;

	friend class TreeImpl;
};


bool serialize(yasli::Archive& ar, TreeSelection &selection, const char* name, const char* label);

// vim:ts=4 sw=4:
