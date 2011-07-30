#include <string>
using std::string;
#include <vector>
using std::vector;

#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/HSplitter.h"
#include "ww/VBox.h"
#include "ww/Button.h"
#include "ww/Color.h"

using yasli::Archive;
using yasli::SharedPtr;
using namespace ww;

enum Filter
{
	FILTER_OUTLINE = 0x1,
	FILTER_PROPERTIES = 0x2
};

struct Node
{
	string name_;
	bool enabled_;
	Color color_;
	float weight_;

	vector<Node> children_;

	Node()
	: enabled_(false)
	, weight_(1.0f)
	{
	}

	void serialize(Archive& ar)
	{
		if (ar.filter(FILTER_OUTLINE)) {
			ar(color_, "color", "^>");
			ar(name_, "name", "^!!");
			ar(children_, "children", "^");
		}

		if (ar.filter(FILTER_PROPERTIES)) {
			if (!ar.filter(FILTER_OUTLINE)) { // prevent duplication when bot filters enabled
				ar(name_, "name", "^");
				ar(color_, "color", "Color");
			}
			ar(enabled_, "enabled", "Enabled");
			ar(weight_, "weight", "Weight");
		}
	}
};

struct TwoTreesData
{
	Node root_;
	ww::signal0 signalChanged_;

	void serialize(Archive& ar)
	{
		ar(root_, "root", "<Tree");
	}

	TwoTreesData()
	{
		generate();
	}

	void generate()
	{
		root_.name_ = "Root";
		root_.children_.clear();
		
		static int index = 0;
		for (int i = 0; i < 5; ++i) {
			char name[32];
			sprintf_s(name, sizeof(name), "Node %d", index);

			Node node;
			node.name_ = name;
			node.color_.setHSV((index % 10) / 9.0f * 360.0f, 1.0f, 1.0f);

			root_.children_.push_back(node);
			++index;
		}

		signalChanged_.emit();
	}

} twoTreesData;

class TwoTreesWidget : public ww::HSplitter
{
public:
	void onGenerate()
	{
		twoTreesData.generate();
	}

	TwoTreesWidget()
	{
		ww::PropertyTree* outlineTree = new ww::PropertyTree();

		ww::VBox* vbox = new ww::VBox();
		{
			outlineTree->setFilter(FILTER_OUTLINE);
			outlineTree->setCompact(true);
			outlineTree->setUndoEnabled(true, false);
			outlineTree->setExpandLevels(2);
			twoTreesData.signalChanged_.connect(outlineTree, &PropertyTree::revert);
			vbox->add(outlineTree, PACK_FILL);

			ww::Button* button = new ww::Button("Generate New Tree", 2);
			button->signalPressed().connect(this, &TwoTreesWidget::onGenerate);
			vbox->add(button, PACK_COMPACT);
		}
		add(vbox, 0.3f);

		ww::PropertyTree* propertyTree = new ww::PropertyTree();
		propertyTree->setFilter(FILTER_PROPERTIES);
		propertyTree->setUndoEnabled(true, false);
		propertyTree->setExpandLevels(2);
		add(propertyTree);

		outlineTree->attachPropertyTree(propertyTree);
		outlineTree->attach(yasli::Serializer(twoTreesData));
	}
};

ww::Widget* createTwoTrees()
{
	ww::HSplitter* splitter = new ww::HSplitter();

	return new TwoTreesWidget();
}

