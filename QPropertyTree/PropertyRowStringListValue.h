/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#pragma once
#include "PropertyRowImpl.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "QPropertyTree.h"
#include <QComboBox>
#include <QStyle>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include "Rect.h"

struct ComboBoxClientRow {
	virtual void populateComboBox(QComboBox* box, QPropertyTree* tree) = 0;
	virtual bool onComboBoxSelected(QComboBox* box, QPropertyTree* tree) = 0;
};

using yasli::StringListValue;
class PropertyRowStringListValue : public PropertyRow, public ComboBoxClientRow {
public:
	InplaceWidget* createWidget(PropertyTree* tree) override;

	void populateComboBox(QComboBox* box, QPropertyTree* tree) override
	{
		for (size_t i = 0; i < stringList_.size(); ++i)
			box->addItem(stringList_[i].c_str());
		box->setCurrentIndex(stringList_.find(value_.c_str()));
	}

	bool onComboBoxSelected(QComboBox* box, QPropertyTree* tree) override
	{
		QByteArray newValue = box->currentText().toUtf8();
		if (value_ != newValue.data()) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = newValue.data();
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}

	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(const Serializer& ser) const override {
		*((StringListValue*)ser.pointer()) = value_.c_str();
		return true;
	}
	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(StringListValue), return);
		const StringListValue& stringListValue = *((StringListValue*)(ser.pointer()));
		stringList_ = stringListValue.stringList();
		value_ = stringListValue.c_str();
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	int widgetSizeMin(const QPropertyTree*) const override { return userWidgetSize() ? userWidgetSize() : 80; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }

	void redraw(PropertyDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
			context.drawComboBox(context.widgetRect, valueAsString().c_str());

	}


	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class InplaceWidgetStringListValue;
};

using yasli::StringListStaticValue;
class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue>, public ComboBoxClientRow {
public:
	InplaceWidget* createWidget(PropertyTree* tree) override;

	void populateComboBox(QComboBox* box, QPropertyTree* tree) override
	{
		for (size_t i = 0; i < stringList_.size(); ++i)
			box->addItem(stringList_[i].c_str());
		box->setCurrentIndex(stringList_.find(value_.c_str()));
	}

	bool onComboBoxSelected(QComboBox* box, QPropertyTree* tree) override
	{
		QByteArray newValue = box->currentText().toUtf8();
		if (value_ != newValue.data()) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = newValue.data();
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}
	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(const Serializer& ser) const override {
		*((StringListStaticValue*)ser.pointer()) = value_.c_str();
		return true;
	}
	void setValueAndContext(const Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(StringListStaticValue), return);
		const StringListStaticValue& stringListValue = *((StringListStaticValue*)(ser.pointer()));
		stringList_.assign(stringListValue.stringList().begin(), stringListValue.stringList().end());
		value_ = stringListValue.c_str();
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	int widgetSizeMin(const QPropertyTree*) const override { return userWidgetSize() ? userWidgetSize() : 80; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }

	void redraw(PropertyDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
			context.drawComboBox(context.widgetRect, valueAsString().c_str());
	}


	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class InplaceWidgetStringListValue;
};
	

class InplaceWidgetComboBox : public QObject, public InplaceWidget
{
	Q_OBJECT
public:
	InplaceWidgetComboBox(PropertyRow* row, ComboBoxClientRow* client, QPropertyTree* tree)
	: InplaceWidget(row, tree)
	, comboBox_(new QComboBox(tree))
	, client_(client)
	, tree_(tree)
	{
		client_->populateComboBox(comboBox_, tree_);
		connect(comboBox_, SIGNAL(activated(int)), this, SLOT(onChange(int)));
	}

	void showPopup() override
	{
		// We could use QComboBox::showPopup() here, but in this case some of
		// the Qt themes (i.e. Fusion) are closing the combobox with following
		// mouse relase because internal timer wasn't fired during the mouse
		// click. We work around this by sending real click to the combo box.
		QSize size = comboBox_->size();
		QPoint centerPoint = QPoint(size.width() * 0.5f, size.height() * 0.5f);
		QMouseEvent ev(QMouseEvent::MouseButtonPress, centerPoint, comboBox_->mapToGlobal(centerPoint), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
		QApplication::sendEvent(comboBox_, &ev);
	}

	~InplaceWidgetComboBox()
	{
		if (comboBox_) {
			comboBox_->hide();
			comboBox_->setParent(0);
			comboBox_->deleteLater();
			comboBox_ = 0;
		}
	}	

	void commit(){}
	void* actualWidget() { return comboBox_; }
	public slots:
	void onChange(int)
	{
		if (!client_->onComboBoxSelected(comboBox_, tree_))
			tree_->_cancelWidget();
	}
protected:
	QComboBox* comboBox_;
	ComboBoxClientRow* client_;
	QPropertyTree* tree_;
};


