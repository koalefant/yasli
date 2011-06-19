#include "ww/Win32/Types.h"
#include "ww/Application.h"
#include "ww/Window.h"

#include "ww/VBox.h"
#include "ww/HBox.h"
#include "ww/VSplitter.h"
#include "ww/HSplitter.h"
#include "ww/ScrolledWindow.h"
#include "ww/Tabs.h"

#include "ww/ImageStore.h"
#include "ww/PropertyTree.h"

#include "ww/Entry.h"
#include "ww/ComboBox.h"
#include "ww/Label.h"
#include "ww/Button.h"
#include "ww/ProgressBar.h"
#include "ww/RadioButton.h"
#include "ww/RadioButtonBox.h"
#include "ww/CheckBox.h"
#include "ww/Frame.h"
#include "ww/Dialog.h"
#include "ww/Serialization.h"
#include "ww/HotkeyDialog.h"
#include "ww/Clipboard.h"
#include "ww/PropertyEditor.h"
#include "ww/Win32Proxy.h"
#include "ww/Win32/Window.h"
#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/BinaryOArchive.h"
#include "yasli/BinaryIArchive.h"
#include "yasli/BinArchive.h"
#include "ExampleLogicEditor.h"

#include "TestData.h"

#include <string>
#include <windows.h>
using std::string;

class MainWindow : public ww::Window{
public:
	MainWindow(ww::Application& app);
	~MainWindow();

protected:
	ww::Application* app_;
};

TestData testData;
TestData testData1;

MainWindow::MainWindow(ww::Application& app)
: app_(&app)
{
	setTitle("wWidgets: Example");
	setBorder(0);
	setMinimizeable(true);
	setResizeable(true);

	setDefaultSize(800, 600);
	setDefaultPosition(ww::POSITION_CENTER);

	signalClose().connect((Win32::MessageLoop*)&app, &Win32::MessageLoop::quit);

	ww::TabPages* pages = new ww::TabPages();
	pages->add("Logic Editor", createLogicEditor());
    add(pages);

	TextIArchive sa;
	sa.setFilter(ww::SERIALIZE_STATE);
	if(sa.load("example.state"))
		sa(*this, "window", "Window");
}

MainWindow::~MainWindow()
{
	TextOArchive oa;
	oa.setFilter(ww::SERIALIZE_STATE);
	oa(*this, "window", "Window");
	oa.save("example.state");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ww::Application app(hInstance);

	SharedPtr<MainWindow> window = new MainWindow(app);
	window->showAll();

	app.run();
	return 0;
}
