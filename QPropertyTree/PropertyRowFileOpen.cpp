/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/ClassFactory.h"

#include "PropertyRowFileOpen.h"
#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/Serialization.h"
#include "PropertyTree/PropertyTreeBase.h"
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/IMenu.h"
#include "yasli/decorators/FileOpen.h"
#include "yasli/decorators/IconXPM.h"
#include <QFileDialog>
#include <QIcon>

static const char* getExtension(const char* path)
{
	const char* ext = strrchr(path, '.');
	if (!ext)
		return path + strlen(path);
	return ext;
}

static yasli::string removeExtension(const char* filename)
{
	const char* ext = getExtension(filename);
	return yasli::string(filename, ext);
}

yasli::string extractExtensionFromFilter(const char* mask)
{
	const char* start = strchr(mask, '(');
	const char* end = strchr(mask, ')');
	if (!start || !end)
		return yasli::string();
	++start;
	const char* ext = strchr(mask, '.');
	if (!ext)
		return yasli::string();
	if (ext > end)
		return yasli::string();
	++ext;
	const char* ext_end = ext;
	while (*ext_end && *ext_end != ' ' && *ext_end != ')' && ext_end < end) {
		if (*ext_end == '*' || *ext_end == '?')
			return yasli::string();
		++ext_end;
	}
	if (ext_end != end)
		++ext_end;
	return yasli::string(ext, ext_end);
}

using yasli::FileOpen;

class PropertyRowFileOpen : public PropertyRowField{
public:

	PropertyRowFileOpen() : flags_() {}

	bool isLeaf() const override{ return true; }

	bool assignTo(const Serializer& ser) const override{
		*ser.cast<FileOpen>()->pathPointer = value_;
		return true;
	}

	void setValueAndContext(const Serializer& ser, Archive& ar) override {
		FileOpen* value = ser.cast<FileOpen>();
		filter_ = value->filter;
		relativeToFolder_ = value->relativeToFolder;
		flags_ = value->flags;
		searchHandle_ = value->pathPointer;
		value_ = value->pathPointer ? *value->pathPointer : "";
	}

	bool onActivate(const PropertyActivationEvent& e) override
	{
		QFileDialog dialog(e.tree->ui()->qwidget());
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		if (labelUndecorated())
			dialog.setWindowTitle(QString("Choose file for '") + labelUndecorated() + "'");		
		
		QDir directory(relativeToFolder_.c_str());
		dialog.setNameFilter(filter_.c_str());
		
		QString existingFile = value_.c_str();
        if (!QDir::isAbsolutePath(existingFile)) {
            existingFile = directory.currentPath();
            if (!relativeToFolder_.empty()) {
                existingFile += QDir::separator();
                existingFile += relativeToFolder_.c_str();
            }
            existingFile += QDir::separator();
            existingFile += value_.c_str();
        }
        if (flags_ & FileOpen::STRIP_EXTENSION)
            existingFile += QString(extractExtensionFromFilter(filter_.c_str()).c_str());

		if (!QFile::exists(existingFile))
			dialog.setDirectory(directory);
		else
            dialog.selectFile(existingFile);

		if (dialog.exec() && !dialog.selectedFiles().isEmpty()) {
			e.tree->model()->rowAboutToBeChanged(this);
			QString filename = dialog.selectedFiles()[0];
			QString relativeFilename = directory.relativeFilePath(filename);
			value_ = relativeFilename.toLocal8Bit().data();
			if (flags_ & FileOpen::STRIP_EXTENSION)
				value_ = removeExtension(value_.c_str());
			e.tree->model()->rowChanged(this);
		}
		return true;
	}

	void clear(PropertyTreeBase* tree)
	{
		tree->model()->rowAboutToBeChanged(this);
		value_.clear();
		tree->model()->rowChanged(this);
	}

	bool onContextMenu(IMenu &menu, PropertyTreeBase* tree) override
	{
		FileOpenMenuHandler* handler = new FileOpenMenuHandler(this, tree);
		tree->addMenuHandler(handler);

		menu.addAction("Choose File...", MENU_DEFAULT, handler, &FileOpenMenuHandler::onMenuActivate);
		menu.addAction("Clear", 0, handler, &FileOpenMenuHandler::onMenuClear);

		return PropertyRow::onContextMenu(menu, tree);
	}

	int buttonCount() const override{ return 1; }
	virtual property_tree::Icon buttonIcon(const PropertyTreeBase* tree, int index) const override{ 
		#include "PropertyTree/file_open.xpm"
		return property_tree::Icon(yasli::IconXPM(file_open_xpm));
	}
	virtual bool usePathEllipsis() const override{ return true; }

	yasli::string valueAsString() const override{ return value_; }
	const void* searchHandle() const override{ return searchHandle_; }
	yasli::TypeID searchType() const override{ return yasli::TypeID::get<yasli::string>(); }
private:
	yasli::string filter_;
	yasli::string relativeToFolder_;
	yasli::string value_;
	int flags_;
	const void* searchHandle_;
};

REGISTER_PROPERTY_ROW(FileOpen, PropertyRowFileOpen); 
DECLARE_SEGMENT(PropertyRowFileOpen)

// ---------------------------------------------------------------------------

void FileOpenMenuHandler::onMenuActivate()
{
	PropertyActivationEvent e;
	e.reason = e.REASON_CONTEXT_MENU;
	e.tree = tree;
	self->onActivate(e);
}

void FileOpenMenuHandler::onMenuClear()
{
	self->clear(tree);
}

