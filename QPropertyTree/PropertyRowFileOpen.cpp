/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/ClassFactory.h"

#include "PropertyDrawContext.h"
#include "PropertyRowImpl.h"
#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "Serialization.h"
#include "yasli/decorators/FileOpen.h"
#include <QtGui/QFileDialog>

using yasli::FileOpen;

class PropertyRowFileOpen : public PropertyRowImpl<FileOpen>{
public:

	bool isLeaf() const override{ return true; }

	bool onActivate(QPropertyTree* tree, bool force)
	{
		QFileDialog dialog(tree);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		if (labelUndecorated())
			dialog.setWindowTitle(QString("Choose file for '") + labelUndecorated() + "'");		
		
		QDir directory(value().relativeToFolder.c_str());
		dialog.setNameFilter(value().filter.c_str());
		
		QString existingFile = value().path.c_str();
		if (!QDir::isAbsolutePath(existingFile))
			existingFile = directory.currentPath() + QDir::separator() + existingFile;

		if (!QFile::exists(existingFile))
			dialog.setDirectory(directory);
		else
			dialog.selectFile(QString(value().path.c_str()));

		if (dialog.exec() && !dialog.selectedFiles().isEmpty()) {
			tree->model()->rowAboutToBeChanged(this);
			QString filename = dialog.selectedFiles()[0];
			QString relativeFilename = directory.relativeFilePath(filename);
			value().path = relativeFilename.toLocal8Bit().data();
			tree->model()->rowChanged(this);
		}
		return true;
	}


	void redraw(const PropertyDrawContext& context)
	{
		if(multiValue())
			context.drawEntry(L" ... ", false, true);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsWString().c_str());
		else
			context.drawEntry(valueAsWString().c_str(), true, false);
	}

	yasli::string valueAsString() const{ return value().path; }
};

REGISTER_PROPERTY_ROW(FileOpen, PropertyRowFileOpen); 
DECLARE_SEGMENT(PropertyRowFileOpen)
