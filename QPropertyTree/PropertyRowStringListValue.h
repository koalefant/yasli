#pragma once
#include "PropertyRowImpl.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "QPropertyTree.h"
#include <QtGui/QComboBox>
#include <QtGui/QStyle>
#include <QtGui/QPainter>


using yasli::StringListValue;
class PropertyRowStringListValue : public PropertyRowImpl<StringListValue, PropertyRowStringListValue>
{
public:
	enum { Custom = true };

	// virtuals:
	PropertyRowWidget* createWidget(QPropertyTree* tree) override;
	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(void* object, size_t size) override {
		*reinterpret_cast<StringListValue*>(object) = value().c_str();
		return true;
	}
	int widgetSizeMin() const override { return userWidgetSize() ? userWidgetSize() : 80; }

	void redraw(const PropertyDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
		{
			QStyleOptionComboBox option;
			option.currentText = QString(valueAsString().c_str());
			option.state |= QStyle::State_Enabled;
			option.rect = context.widgetRect;
			context.tree->style()->drawComplexControl(QStyle::CC_ComboBox, &option, context.painter);
			context.painter->setPen(QPen(context.tree->palette().color(QPalette::WindowText)));
			QRect textRect = context.tree->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, 0);
			textRect.adjust(1, 0, -1, 0);
			context.tree->_drawRowValue(*context.painter, valueAsWString().c_str(), &context.tree->font(), textRect, context.tree->palette().color(QPalette::WindowText), false, false);

		}
	}
};

using yasli::StringListStaticValue;
class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>{
public:
	enum { Custom = false };

	PropertyRowWidget* createWidget(QPropertyTree* tree) override;
	yasli::string valueAsString() const  override{ return value_.c_str(); }
	bool assignTo(void* object, size_t size) override{
		*reinterpret_cast<StringListStaticValue*>(object) = value().index();
		return true;
	}
	int widgetSizeMin() const override{ return userWidgetSize() >= 0 ? userWidgetSize() : 80; }
	
	void redraw(const PropertyDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
		{
			QStyleOptionComboBox option;
			option.currentText = QString(valueAsString().c_str());
			option.state |= QStyle::State_Enabled;
			option.rect = context.widgetRect;
			context.tree->style()->drawComplexControl(QStyle::CC_ComboBox, &option, context.painter);
			context.painter->setPen(QPen(context.tree->palette().color(QPalette::WindowText)));
			QRect textRect = context.tree->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, 0);
			textRect.adjust(1, 0, -1, 0);
			context.tree->_drawRowValue(*context.painter, valueAsWString().c_str(), &context.tree->font(), textRect, context.tree->palette().color(QPalette::WindowText), false, false);
		}
	}
};

class QAutoComboBox : public QComboBox
{
public:
	QAutoComboBox() : firstShow_(true), hiding_(false) {}
protected:
	void hidePopup() override
	{
		QComboBox::hidePopup();
		hiding_ = true;
	}

	void focusOutEvent(QFocusEvent* ev)
	{
		QComboBox::focusOutEvent(ev);
		if (hiding_)
		{
			currentIndexChanged(currentIndex());
			hiding_ = false;
		}
	}

	void focusInEvent(QFocusEvent* ev) override
	{
		QComboBox::focusInEvent(ev);
		if (firstShow_) {
			showPopup();
			firstShow_ = false;
		}
	}
	bool firstShow_;
	bool hiding_;
};

class PropertyRowWidgetStringListValue : public PropertyRowWidget
{
	Q_OBJECT
public:
	PropertyRowWidgetStringListValue(PropertyRowStringListValue* row, QPropertyTree* tree)
	: PropertyRowWidget(row, tree)
	, comboBox_(new QAutoComboBox())
	{
		const StringListValue& slv = row->value();
		for (size_t i = 0; i < slv.stringList().size(); ++i)
			comboBox_->addItem(slv.stringList()[i].c_str());
		comboBox_->setCurrentIndex(slv.index());
		connect(comboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(onChange(int)));
	}

	PropertyRowWidgetStringListValue(PropertyRowStringListStaticValue* row, QPropertyTree* tree)
	: PropertyRowWidget(row, tree)
	, comboBox_(new QAutoComboBox())
	{
		const StringListStaticValue& slv = row->value();
		for (size_t i = 0; i < slv.stringList().size(); ++i)
			comboBox_->addItem(slv.stringList()[i]);
		comboBox_->setCurrentIndex(slv.index());
		connect(comboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(onChange(int)));
	}

	~PropertyRowWidgetStringListValue()
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
			if ( strcmp(this->row()->typeName(), yasli::TypeID::get<StringListValue>().name()) == 0 )
			{
				PropertyRowStringListValue* row = static_cast<PropertyRowStringListValue*>(this->row());
				model()->push(row);
				StringListValue comboList = row->value();
				comboList = comboBox_->currentIndex();
				//getStringListValueFromComboBox(&comboList, comboBox_);
				row->setValue(comboList);
				model()->rowChanged(row);
			}
			else if ( strcmp(this->row()->typeName(), yasli::TypeID::get<StringListStaticValue>().name()) == 0 )
			{
				PropertyRowStringListStaticValue* row = static_cast<PropertyRowStringListStaticValue*>(this->row());
				model()->push(row);
				StringListStaticValue comboList = row->value();
				comboList = comboBox_->currentIndex();
				row->setValue(comboList);
				model()->rowChanged(row);
			}
		}
protected:
	QComboBox* comboBox_;
};

