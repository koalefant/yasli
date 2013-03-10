#include "mainwindow.h"
#include "QPropertyTree/QPropertyTree.h"
#include <limits.h>
#include <float.h>

#include "yasli/STL.h"
#include "yasli/Archive.h"
using yasli::Archive;
#include "yasli/STLImpl.h"

struct Vec3
{
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}

	void serialize(Archive& ar)
	{
		ar(x, "", "^");
		ar(y, "", "^");
		ar(z, "", "^");
	}
};

struct Element
{
    Element()
	: enabled(false)
	, char_(255)
	, signedChar_(127)
	, unsignedChar_(255)
	, short_(SHRT_MAX)
	, unsignedShort_(USHRT_MAX)
	, unsignedInteger_(UINT_MAX)
	, integer_(INT_MAX)
	, float_(FLT_MAX)
	, double_(DBL_MAX)
	{}

    void serialize(Archive& ar)
	{
		ar(enabled, "enabled", "&Enabled");
		ar(vec, "vec", "&Vec3");
		ar(integer_, "integer_", "&integer_");
		ar(char_, "char", "char");
		ar(signedChar_, "signedChar", "signedChar");
		ar(unsignedChar_, "unsignedChar", "unsignedChar");
		ar(unsignedInteger_, "unsignedInteger", "unsignedInteger");
		ar(short_, "short", "short");
		ar(unsignedShort_, "unsignedShort", "unsignedShort");
		ar(longInteger_, "longInteger", "longInteger");
		ar(unsignedLongInteger_, "unsignedLongInteger", "unsignedLongInteger");
		ar(float_, "float", "float");
		ar(double_, "double", "double");
	}

    bool enabled;
	Vec3 vec;
    float x, y, z;
	int integer_;
	char char_;
	signed char signedChar_;
	unsigned char unsignedChar_;
	unsigned int unsignedInteger_;
	short short_;
	unsigned short unsignedShort_;
	long long longInteger_;
	unsigned long long unsignedLongInteger_;
	float float_;
	double double_;
};

struct TestData
{
    vector<Element> elements;
	Vec3 vec3;
    void serialize(Archive& ar)
    {
		//ar(vec3, "vec3", "Vec3");
        ar(elements, "elements", "Elements");
    }
} globalTestData;

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
#ifdef NDEBUG
	globalTestData.elements.resize(50000);
#else
	globalTestData.elements.resize(2500);
#endif
    tree_ = new QPropertyTree(this);
    tree_->attach(yasli::Serializer(globalTestData));
    setCentralWidget(tree_);
	tree_->setUndoEnabled(true);

	connect(tree_, SIGNAL(signalChanged()), this, SLOT(onPropertyChanged()));
}

void MainWindow::onPropertyChanged()
{
	QString title = QString("Test: %1(a)+%2(r)+%3(h)+%4(p)")
		.arg(tree_->_applyTime())
		.arg(tree_->_revertTime())
		.arg(tree_->_updateHeightsTime())
		.arg(tree_->_paintTime());
	QMainWindow::setWindowTitle(title);
}

MainWindow::~MainWindow()
{
}
