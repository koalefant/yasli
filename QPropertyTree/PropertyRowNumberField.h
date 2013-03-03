/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "PropertyRow.h"
#include <QtGui/QLineEdit>

class PropertyRowNumberField;
class PropertyRowWidgetNumber : public PropertyRowWidget
{
	Q_OBJECT
public:
	PropertyRowWidgetNumber(PropertyTreeModel* mode, PropertyRowNumberField* numberField, QPropertyTree* tree);
	~PropertyRowWidgetNumber(){
		if (entry_)
			entry_->setParent(0);
		entry_->deleteLater();
		entry_ = 0;
	}

	void commit();
	QWidget* actualWidget() { return entry_; }
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
	PropertyRowNumberField();
	PropertyRowNumberField(const char* name, const char* nameAlt, const char* typeName);
	PropertyRowNumberField(const char* name, const char* nameAlt, const yasli::Serializer& ser);

	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	int widgetSizeMin() const{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 40;
	}

	PropertyRowWidget* createWidget(QPropertyTree* tree) override;
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	void redraw(const PropertyDrawContext& context) override;

	virtual void incrementLog(float screenFraction) {}
	virtual bool setValueFromString(const char* str) = 0;
};

