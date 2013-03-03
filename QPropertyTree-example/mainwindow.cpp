#include "mainwindow.h"
#include "QPropertyTree/QPropertyTree.h"
#include <limits.h>
#include <float.h>

#include "yasli/STL.h"
#include "yasli/Archive.h"
using yasli::Archive;
#include "yasli/STLImpl.h"

struct Element
{
    Element()
	: enabled(false)
	, x(0.0f)
	, y(0.0f)
	, z(0.0f)
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
		ar(enabled, "enabled", "Enabled");
		ar(x, "x", "&X element");
		ar(y, "y", "&Y element");
		ar(z, "z", "&Z element");
		ar(integer_, "integer_", "integer_");
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
    void serialize(Archive& ar)
    {
        ar(elements, "elements", "Elements");
    }
} globalTestData;

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
    QPropertyTree* tree = new QPropertyTree(this);
    tree->attach(yasli::Serializer(globalTestData));
    setCentralWidget(tree);
}

MainWindow::~MainWindow()
{
}
