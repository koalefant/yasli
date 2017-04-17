/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/ClassFactory.h"

#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "PropertyTree/PropertyTreeBase.h"
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/Serialization.h"
#include "yasli/decorators/FileSave.h"
#include "yasli/decorators/IconXPM.h"
#include <QFileDialog>
#include <QIcon>

#include "PropertyRowFileOpen.h"

using yasli::FileSave;

class PropertyRowFileSave : public PropertyRowField{
public:

	bool assignTo(const Serializer& ser) const override{
		*ser.cast<FileSave>() = value_;
		return true;
	}
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }

	void setValueAndContext(const Serializer& ser, Archive& ar) override {
		FileSave* value = ser.cast<FileSave>();
		value_ = *value;
		searchHandle_ = value->pathPointer;
	}

	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
	}
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }

	bool onActivate(const PropertyActivationEvent& e) override
	{
		QFileDialog dialog(e.tree->ui()->qwidget());
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setDefaultSuffix(extractExtensionFromFilter(value_.filter.c_str()).c_str());
		if (labelUndecorated())
			dialog.setWindowTitle(QString("Choose file for '") + labelUndecorated() + "'");		
		
		QDir directory(value_.relativeToFolder.c_str());
		dialog.setNameFilter(value_.filter.c_str());
		
		QString existingFile = value_.path.c_str();
		if (!QDir::isAbsolutePath(existingFile))
			existingFile = directory.currentPath() + QDir::separator() + existingFile;

		if (!QFile::exists(existingFile))
			dialog.setDirectory(directory);
		else
			dialog.selectFile(QString(value_.path.c_str()));

		if (dialog.exec() && !dialog.selectedFiles().isEmpty()) {
			e.tree->model()->rowAboutToBeChanged(this);
			QString filename = dialog.selectedFiles()[0];
			QString relativeFilename = directory.relativeFilePath(filename);
			value_.path = relativeFilename.toLocal8Bit().data();
			e.tree->model()->rowChanged(this);
		}
		return true;
	}


	int buttonCount() const override{ return 1; }
	property_tree::Icon buttonIcon(const PropertyTreeBase* tree, int index) const override{ 
		#include "PropertyTree/file_save.xpm"
		return property_tree::Icon(yasli::IconXPM(file_save_xpm));
	}
	virtual bool usePathEllipsis() const override{ return true; }

	yasli::string valueAsString() const override{ return value_.path; }
	const void* searchHandle() const override { return searchHandle_; }
	yasli::TypeID searchType() const override { return yasli::TypeID::get<yasli::string>(); }
	yasli::TypeID typeId() const override { return yasli::TypeID::get<yasli::string>(); }
private:
	FileSave value_;
	const void* searchHandle_;
};

REGISTER_PROPERTY_ROW(FileSave, PropertyRowFileSave); 
DECLARE_SEGMENT(PropertyRowFileSave)
