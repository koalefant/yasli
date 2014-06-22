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

struct ComboBoxClientRow {
	virtual void populateComboBox(QComboBox* box, QPropertyTree* tree) = 0;
	virtual bool onComboBoxSelected(QComboBox* box, QPropertyTree* tree) = 0;
};

using yasli::StringListValue;
class PropertyRowStringListValue : public PropertyRow, public ComboBoxClientRow {
public:
	PropertyRowWidget* createWidget(QPropertyTree* tree) override;

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

	void redraw(const PropertyDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
		{
			QStyleOptionComboBox option;
			option.editable = false;
			option.frame = true;
			option.currentText = QString(valueAsString().c_str());
			option.state |= QStyle::State_Enabled;
			option.rect = QRect(0, 0, context.widgetRect.width(), context.widgetRect.height());
			// we have to translate painter here to work around bug in some themes
			context.painter->translate(context.widgetRect.left(), context.widgetRect.top());
			context.tree->style()->drawComplexControl(QStyle::CC_ComboBox, &option, context.painter);
			context.painter->setPen(QPen(context.tree->palette().color(QPalette::WindowText)));
			QRect textRect = context.tree->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, 0);
			textRect.adjust(1, 0, -1, 0);
			context.tree->_drawRowValue(*context.painter, valueAsWString().c_str(), &context.tree->font(), textRect, context.tree->palette().color(QPalette::WindowText), false, false);
			context.painter->translate(-context.widgetRect.left(), -context.widgetRect.top());
		}
	}


	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class PropertyRowWidgetStringListValue;
};

using yasli::StringListStaticValue;
class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue>, public ComboBoxClientRow {
public:
	PropertyRowWidget* createWidget(QPropertyTree* tree) override;

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

	void redraw(const PropertyDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
		{
			QStyleOptionComboBox option;
			option.editable = false;
			option.frame = true;
			option.currentText = QString(valueAsString().c_str());
			option.state |= QStyle::State_Enabled;
			option.rect = QRect(0, 0, context.widgetRect.width(), context.widgetRect.height());
			// we have to translate painter here to work around bug in some themes
			context.painter->translate(context.widgetRect.left(), context.widgetRect.top());
			context.tree->style()->drawComplexControl(QStyle::CC_ComboBox, &option, context.painter);
			context.painter->setPen(QPen(context.tree->palette().color(QPalette::WindowText)));
			QRect textRect = context.tree->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, 0);
			textRect.adjust(1, 0, -1, 0);
			context.tree->_drawRowValue(*context.painter, valueAsWString().c_str(), &context.tree->font(), textRect, context.tree->palette().color(QPalette::WindowText), false, false);
			context.painter->translate(-context.widgetRect.left(), -context.widgetRect.top());
		}
	}


	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class PropertyRowWidgetStringListValue;
};
	

class PropertyRowWidgetComboBox : public PropertyRowWidget
{
	Q_OBJECT
public:
	PropertyRowWidgetComboBox(PropertyRow* row, ComboBoxClientRow* client, QPropertyTree* tree)
	: PropertyRowWidget(row, tree)
	, comboBox_(new QComboBox())
	, client_(client)
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

	~PropertyRowWidgetComboBox()
	{
		comboBox_->hide();
		comboBox_->setParent(0);
		comboBox_->deleteLater();
		comboBox_ = 0;
	}	

	void commit(){}
	QWidget* actualWidget() { return comboBox_; }
	public slots:
	void onChange(int)
	{
		if (!client_->onComboBoxSelected(comboBox_, tree_))
			tree_->_cancelWidget();
	}
protected:
	QComboBox* comboBox_;
	ComboBoxClientRow* client_;
};


