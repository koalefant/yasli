#pragma once
#include <QObject>
#include <QLineEdit>
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyRowString.h"

class InplaceWidgetString : public QObject, public property_tree::InplaceWidget
{
	Q_OBJECT
public:
  InplaceWidgetString(PropertyRowString* row, QPropertyTree* tree)
	: InplaceWidget(tree)
	, entry_(new QLineEdit(tree))
	, tree_(tree)
	, row_(row)
	{
		initialValue_ = row->value().c_str();
		entry_->setText(initialValue_);
		entry_->selectAll();
		QObject::connect(entry_.data(), SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	}
	~InplaceWidgetString()
	{
		entry_->hide();
		entry_->setParent(0);
		entry_.take()->deleteLater();
	}

	void commit(){
		onEditingFinished();
	}
	void* actualWidget() { return entry_.data(); }

	public slots:
	void onEditingFinished(){
		if(initialValue_ != entry_->text() || row_->multiValue()){
			tree()->model()->rowAboutToBeChanged(row_);
			QString text = entry_->text();
			row_->setValue(text.toUtf8().data());
			tree()->model()->rowChanged(row_);
		}
		else
			tree_->_cancelWidget();
	}
protected:
    QPropertyTree* tree_;
	QScopedPointer<QLineEdit> entry_;
	PropertyRowString* row_;
    QString initialValue_;
};
