#include "mainwindow.h"
#include "QPropertyTree/QPropertyTree.h"


#include "yasli/STL.h"
#include "yasli/Archive.h"
using yasli::Archive;
#include "yasli/STLImpl.h"

struct Element
{
    Element() : enabled(false), x(0.0f), y(0.0f), z(0.0f) {}
    void serialize(Archive& ar)
    {
        ar(enabled, "enabled", "Enabled");
        ar(x, "x", "&X element");
        ar(y, "y", "&Y element");
        ar(z, "z", "&Z element");
    }

    bool enabled;
    float x, y, z;
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
