
#include <string>
#include <vector>
using std::vector;
#include "ExampleStyle.h"

#include "QPropertyTree/QPropertyTree.h"
#include "PropertyTree/PropertyTreeStyle.h"
#include "PropertyTree/Color.h"
#include "yasli/STL.h"
#include "yasli/decorators/IconXPM.h"
#include <QSplitter>
#include <QPushButton>
#include <QBoxLayout>

using yasli::Archive;
using yasli::SharedPtr;

struct ExampleStyleContent
{
	bool options[13];

	std::string stringValue;
	int integerValue;
	float floatValue;

	ExampleStyleContent() 
	: stringValue("String")
	, integerValue(13)
	, floatValue(50.0f)
	{
		for ( int i = 0; i < sizeof(options)/sizeof(options[0]); ++i ) {
			options[i] = true;
		}
	}

	void serialize(Archive& ar)
	{
		ar.warning(*this, "Note: you can use Ctrl+Mouse Wheel to zoom into property tree.");

		// Note that arrays are fully supported with
		// ar(options, "options", "Options") call,
		// we just need a bunch of individual flags here
		// to test two-columns packing.
		if (ar.openBlock("twoColumns", "Two Columns")) {
			ar(options[0], "option0", "First Option");
			ar(options[1], "option1", "Second Option");
			ar(options[2], "option2", "Third Option");
			ar(options[3], "option3", "Fourth Option");
			ar(options[4], "option4", "Fifth Option");
			ar(options[5], "option5", "Option Six");
			ar.warning(options[5], "Long bubble with descriptive text that spans accross multiple lines");
			ar(options[6], "option6", "Seven");
			ar(options[7], "option7", "Eight");
			ar(options[8], "option8", "Nein");
			ar(options[9], "option9", "Ten");
			if (ar.openBlock("nested", "Nested")) {
			ar(options[10], "option10", "Option Eleven");
			ar(options[11], "option11", "Twelve");
			ar(options[12], "option12", "Thirteen");
			ar.closeBlock();
			}
			ar.closeBlock();
		}

		if (ar.openBlock("level1", "Level 1")) {
			if (ar.openBlock("level2", "Level 2")) {
				if (ar.openBlock("level3", "+Level 3")) {
					ar(stringValue, "stringValue", "^String");
					ar(integerValue, "integerValue", "^Integer");
					ar.warning(integerValue,
						"Numeric values can be scrolled with the mouse "
						"even if there is no range set. Try holding mouse "
						"button and dragging it from one side to another");
					ar.closeBlock();
				}
				ar.closeBlock();
			}
			ar(yasli::Range(floatValue, 0.0f, 100.0f), "floatValue", "Float Slider");
			ar.closeBlock();
		}
	}

} exampleStyleContent;

PropertyTreeStyle exampleStyle;

void ExampleStyleWidget::onContentChanged()
{
}

void ExampleStyleWidget::onStyleChanged()
{
	contentTree->setTreeStyle(exampleStyle);
}

ExampleStyleWidget::ExampleStyleWidget()
: QSplitter(Qt::Horizontal)
{
	contentTree = new QPropertyTree();
	contentTree->setUndoEnabled(true, false);
	QObject::connect(contentTree, SIGNAL(signalChanged(bool)), this, SLOT(onContentChanged()));
	contentTree->setExpandLevels(2);

	addWidget(contentTree);

	QBoxLayout* vbox = new QBoxLayout(QBoxLayout::TopToBottom);
	QWidget* rightWidget = new QWidget();
	{
		rightWidget->setLayout(vbox);
		styleTree = new QPropertyTree();
		styleTree->setUndoEnabled(true, false);
		styleTree->setValueColumnWidth(0.5f);
		styleTree->setExpandLevels(2);
		QObject::connect(styleTree, SIGNAL(signalChanged()), this, SLOT(onStyleChanged()));
		QObject::connect(styleTree, SIGNAL(signalContinuousChange()), this, SLOT(onStyleChanged()));
		vbox->addWidget(styleTree, 1);

		QPushButton* button = new QPushButton("Reset Style");
		QObject::connect(button, SIGNAL(pressed()), this, SLOT(onResetStyle()));
		vbox->addWidget(button, 0);
	}
	addWidget(rightWidget);

	contentTree->attach(yasli::Serializer(exampleStyleContent));
	styleTree->attach(yasli::Serializer(exampleStyle));
}

void ExampleStyleWidget::onResetStyle()
{
	exampleStyle = PropertyTreeStyle();
	styleTree->revert();

	contentTree->setTreeStyle(exampleStyle);
	contentTree->update();
}

QWidget* createExampleStyle()
{
	return new ExampleStyleWidget();
}

