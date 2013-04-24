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
#include "yasli/decorators/FileSave.h"
#include "yasli/decorators/IconXPM.h"
#include <QtGui/QFileDialog>
#include <QtGui/QIcon>

using yasli::FileSave;

class PropertyRowFileSave : public PropertyRowImpl<FileSave>{
public:

	bool isLeaf() const override{ return true; }

	bool onActivate(QPropertyTree* tree, bool force) override
	{
		QFileDialog dialog(tree);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setFileMode(QFileDialog::AnyFile);
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


	int buttonCount() const override{ return 1; }
	virtual const QIcon& buttonIcon(const QPropertyTree* tree, int index) const override{ 
		#include "file_save.xpm"
		static QIcon icon = QIcon(QPixmap::fromImage(*tree->_iconCache()->getImageForIcon(yasli::IconXPM(file_save_xpm))));
		return icon;
	}
	virtual bool usePathEllipsis() const override{ return true; }

	yasli::string valueAsString() const{ return value().path; }
};

REGISTER_PROPERTY_ROW(FileSave, PropertyRowFileSave); 
DECLARE_SEGMENT(PropertyRowFileSave)
