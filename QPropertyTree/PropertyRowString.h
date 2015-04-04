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

#include <QObject>
#include <QWidget>
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
	InplaceWidget* createWidget(PropertyTree* tree);
	yasli::string valueAsString() const override;
	yasli::wstring valueAsWString() const override { return value_; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	void serializeValue(yasli::Archive& ar) override;
	const yasli::wstring& value() const{ return value_; }
private:
	yasli::wstring value_;
};

class InplaceWidgetString : public QObject, public InplaceWidget
{
	Q_OBJECT
public:
  InplaceWidgetString(PropertyRowString* row, QPropertyTree* tree)
	: InplaceWidget(row, tree)
	, entry_(new QLineEdit(tree))
	, tree_(tree)
	{
#ifdef _MSC_VER
		initialValue_ = QString::fromUtf16((const ushort*)row->value().c_str());
#else
		initialValue_ = QString::fromWCharArray(row->value().c_str());
#endif
		entry_->setText(initialValue_);
		entry_->selectAll();
		QObject::connect(entry_.data(), SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	}
	~InplaceWidgetString()
	{
		entry_->hide();
		entry_->setParent(0);
		entry_.take()->deleteLater();
	}

	void commit(){
		onEditingFinished();
	}
	void* actualWidget() { return entry_.data(); }

	public slots:
	void onEditingFinished(){
		PropertyRowString* row = static_cast<PropertyRowString*>(this->row());
		if(initialValue_ != entry_->text() || row_->multiValue()){
			model()->rowAboutToBeChanged(row);
			vector<wchar_t> str;
			QString text = entry_->text();
#ifdef _MSC_VER
			str.assign((const wchar_t*)text.utf16(), (const wchar_t*)(text.utf16() + text.size() + 1));
#else
			str.resize(text.size() + 1, L'\0');
			if (!text.isEmpty())
				text.toWCharArray(&str[0]);
#endif
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
