/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "PropertyRowField.h"
#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "Unicode.h"

#include <QLineEdit>

class PropertyRowString : public PropertyRowField
{
public:
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	bool assignTo(yasli::string& str);
	bool assignTo(yasli::wstring& str);
	void setValue(const char* str);
	void setValue(const wchar_t* str);
	PropertyRowWidget* createWidget(QPropertyTree* tree);
	yasli::string valueAsString() const override;
	yasli::wstring valueAsWString() const override { return value_; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	void serializeValue(yasli::Archive& ar) override;
	const yasli::wstring& value() const{ return value_; }
private:
	yasli::wstring value_;
};

class PropertyRowWidgetString : public PropertyRowWidget
{
	Q_OBJECT
public:
  PropertyRowWidgetString(PropertyRowString* row, QPropertyTree* tree)
	: PropertyRowWidget(row, tree)
	, entry_(new QLineEdit())
	, tree_(tree)
	{
        initialValue_ = QString(fromWideChar(row->value().c_str()).c_str());
		entry_->setText(initialValue_);
		entry_->selectAll();
		connect(entry_.data(), SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	}
	~PropertyRowWidgetString()
	{
		entry_->hide();
		entry_->setParent(0);
		entry_.take()->deleteLater();
	}

	void commit(){
		onEditingFinished();
	}
	QWidget* actualWidget() { return entry_.data(); }

	public slots:
	void onEditingFinished(){
		PropertyRowString* row = static_cast<PropertyRowString*>(this->row());
		if(initialValue_ != entry_->text() || row_->multiValue()){
			model()->rowAboutToBeChanged(row);
			vector<wchar_t> str;
			QString text = entry_->text();
			str.resize(text.size() + 1, L'\0');
			if (!text.isEmpty())
				text.toWCharArray(&str[0]);
			row->setValue(&str[0]);
			model()->rowChanged(row);
		}
		else
			tree_->_cancelWidget();
	}
protected:
    QPropertyTree* tree_;
	QScopedPointer<QLineEdit> entry_;
    QString initialValue_;
};
