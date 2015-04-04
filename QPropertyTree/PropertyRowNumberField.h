/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "PropertyRow.h"
#include <QLineEdit>

class QPropertyTree;
class PropertyRowNumberField;
class InplaceWidgetNumber : public QObject, public InplaceWidget
{
	Q_OBJECT
public:
	InplaceWidgetNumber(PropertyTreeModel* mode, PropertyRowNumberField* numberField, QPropertyTree* tree);
	~InplaceWidgetNumber(){
		if (entry_)
			entry_->setParent(0);
		entry_->deleteLater();
		entry_ = 0;
	}

	void commit();
	void* actualWidget() { return entry_; }
public slots:
	void onEditingFinished();
protected:
	QLineEdit* entry_;
	PropertyRowNumberField* row_;
	QPropertyTree* tree_;
};

// ---------------------------------------------------------------------------

class PropertyRowNumberField : public PropertyRow
{
public:
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	int widgetSizeMin(const PropertyTree* tree) const override{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 40;
	}

	InplaceWidget* createWidget(PropertyTree* tree) override;
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	void redraw(PropertyDrawContext& context) override;
	bool onActivate(PropertyTree* tree, bool force) override;
	bool onActivateRelease(PropertyTree* tree) override;
	bool onMouseDown(PropertyTree* tree, Point point, bool& changed) override;
	void onMouseUp(PropertyTree* tree, Point point) override;
	void onMouseDrag(const PropertyDragEvent& e) override;
	void onMouseStill(const PropertyDragEvent& e) override;

	virtual void startIncrement() = 0;
	virtual void endIncrement(PropertyTree* tree) = 0;
	virtual void incrementLog(float screenFraction) = 0;
	virtual bool setValueFromString(const char* str) = 0;
};

