/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/BinaryOArchive.h"
#include "yasli/BinaryIArchive.h"
#include "yasli/PointersImpl.h"
#include "QPropertyTree.h"
#include "PropertyDrawContext.h"
#include "Serialization.h"
#include "PropertyTreeModel.h"

#include "yasli/ClassFactory.h"

#include "PropertyOArchive.h"
#include "PropertyIArchive.h"
#include "Unicode.h"

#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtCore/QElapsedTimer>
#include "PropertyTreeMenuHandler.h"

// only for clipboard:
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include "PropertyRowPointer.h"
#include "PropertyRowContainer.h"
// ^^^

static int min(int a, int b)
{
	return a < b ? a : b;
}

static int max(int a, int b)
{
	return a > b ? a : b;
}

using yasli::Serializers;

void PropertyTreeMenuHandler::onMenuFilterByName()
{
	tree->startFilter(filterName.c_str());
}

void PropertyTreeMenuHandler::onMenuFilterByValue()
{
	tree->startFilter(filterValue.c_str());
}

void PropertyTreeMenuHandler::onMenuFilterByType()
{
	tree->startFilter(filterType.c_str());
}

void PropertyTreeMenuHandler::onMenuUndo()
{
	tree->model()->undo();
}

static QMimeData* propertyRowToMimeData(PropertyRow* row, ConstStringList* constStrings)
{
	PropertyRow::setConstStrings(constStrings);
	SharedPtr<PropertyRow> clonedRow(row->clone());
	yasli::BinaryOArchive oa(true);
	PropertyRow::setConstStrings(constStrings);
	if (!oa(clonedRow, "row", "Row")) {
		PropertyRow::setConstStrings(0);
		return false;
	}
	PropertyRow::setConstStrings(0);

	QByteArray byteArray(oa.buffer(), oa.length());
	QMimeData* mime = new QMimeData;
	mime->setData("binary/qpropertytree", byteArray);
	return mime;
}

static bool smartPaste(PropertyRow* dest, SharedPtr<PropertyRow>& source, PropertyTreeModel* model, bool onlyCheck)
{
	bool result = false;
	// FIXME: typeName is generated inproperly here, as it will fail to provide
	// match for vector<vector<> >.
	if(strcmp(dest->typeName(), source->typeName()) == 0 && 
		source->isContainer() == dest->isContainer()){
		result = true;
		if(!onlyCheck){
			if(dest->isPointer() && !source->isPointer()){
				PropertyRowPointer* d = static_cast<PropertyRowPointer*>(dest);

				const char* derivedName = d->typeName();
				const char* derivedNameAlt = d->typeName();
				PropertyRowPointer* newSourceRoot = static_cast<PropertyRowPointer*>(d->clone());
					// new PropertyRowPointer(d->name(), d->label(), d->baseType(), d->factory(), d->derivedTypeName());
				source->swapChildren(newSourceRoot);
				source = newSourceRoot;
			}
			const char* name = dest->name();
			const char* nameAlt = dest->label();
			source->setName(name);
			source->setLabel(nameAlt);
			if(dest->parent())
				dest->parent()->replaceAndPreserveState(dest, source, false);
			else{
				dest->clear();
				dest->swapChildren(source);
			}
			source->setLabelChanged();
		}
	}
	else if(dest->isContainer()){
		if(model){
			PropertyRowContainer* container = static_cast<PropertyRowContainer*>(dest);
			PropertyRow* elementRow = model->defaultType(container->elementTypeName());
			YASLI_ESCAPE(elementRow, return false);
			if(strcmp(elementRow->typeName(), source->typeName()) == 0){
				result = true;
				if(!onlyCheck){
					PropertyRow* dest = elementRow;
					if(dest->isPointer() && !source->isPointer()){
						PropertyRowPointer* d = static_cast<PropertyRowPointer*>(dest);

						const char* derivedName = d->typeName();
						const char* derivedNameAlt = d->typeName();
						PropertyRowPointer* newSourceRoot = static_cast<PropertyRowPointer*>(d->clone());
						//new PropertyRowPointer(d->name(), d->label(), d->typeName(), d->factory(), d->derivedTypeName()
						source->swapChildren(newSourceRoot);
						source = newSourceRoot;
					}

					container->add(source.get());
				}
			}
			container->setLabelChanged();
		}
	}
	
	return result;
}

static bool propertyRowFromMimeData(SharedPtr<PropertyRow>& row, const QMimeData* mimeData, ConstStringList* constStrings)
{
	PropertyRow::setConstStrings(constStrings);
	QStringList formats = mimeData->formats();
	QByteArray array = mimeData->data("binary/qpropertytree");
	if (array.isEmpty())
		return 0;
	yasli::BinaryIArchive ia(true);
	if (!ia.open(array.data(), array.size()))
		return 0;

	if (!ia(row, "row", "Row"))
		return false;

	PropertyRow::setConstStrings(0);
	return true;

}

bool propertyRowFromClipboard(SharedPtr<PropertyRow>& row, ConstStringList* constStrings)
{
	const QMimeData* mime = QApplication::clipboard()->mimeData();
	if (!mime)
		return false;
	return propertyRowFromMimeData(row, mime, constStrings);
}

void PropertyTreeMenuHandler::onMenuCopy()
{
	QMimeData* mime = propertyRowToMimeData(row, tree->constStrings());
	if (mime)
		QApplication::clipboard()->setMimeData(mime);
}

void PropertyTreeMenuHandler::onMenuPaste()
{
	if(!tree->canBePasted(row))
		return;
	PropertyRow* parent = row->parent();

	tree->model()->push(row);

	SharedPtr<PropertyRow> source;
	if (!propertyRowFromClipboard(source, tree->constStrings()))
		return;

	if (!smartPaste(row, source, tree->model(), false))
		return;
	
	tree->model()->rowChanged(parent ? parent : tree->model()->root());
}

class FilterEntry : public QLineEdit
{
public:
	FilterEntry(QPropertyTree* tree)
    : QLineEdit(tree)
    , tree_(tree)
	{
        //setSwallowArrows(true);
        //setSwallowReturn(true);
        //setSwallowEscape(true);
	}
protected:

    void keyPressEvent(QKeyEvent * ev)
    {
        if (ev->key() == Qt::Key_Escape || ev->key() == Qt::Key_Return)
        {
            ev->accept();
            tree_->setFocus();
            tree_->keyPressEvent(ev);
        }

        if (ev->key() == Qt::Key_Backspace && text()[0] == '\0')
        {
            tree_->setFilterMode(false);
        }
        QLineEdit::keyPressEvent(ev);
    }
private:
	QPropertyTree* tree_;
};


// ---------------------------------------------------------------------------

TreeConfig TreeConfig::defaultConfig;

TreeConfig::TreeConfig()
: immediateUpdate_(true)
, hideUntranslated_(true)
, valueColumnWidth_(.5f)
, filter_(YASLI_DEFAULT_FILTER)
, compact_(false)
, fullRowMode_(false)
, showContainerIndices_(true)
, filterWhenType_(true)
, tabSize_(PropertyRow::ROW_DEFAULT_HEIGHT)
, sliderUpdateDelay_(25)
{
}

// ---------------------------------------------------------------------------

DragWindow::DragWindow(QPropertyTree* tree)
: tree_(tree)
, offset_(0, 0)
{
	QWidget::setWindowFlags(Qt::ToolTip);
	QWidget::setWindowOpacity(192.0f/ 256.0f);
}

void DragWindow::set(QPropertyTree* tree, PropertyRow* row, const QRect& rowRect)
{
	QRect rect = tree->rect();
	rect.setTopLeft(tree->mapToGlobal(rect.topLeft()));

	offset_ = rect.topLeft();

	row_ = row;
	rect_ = rowRect;
}

void DragWindow::setWindowPos(bool visible)
{
	QWidget::move(rect_.left() + offset_.x() - 1,  rect_.top() + offset_.y() - 1);
	QWidget::resize(rect_.width() + 2, rect_.height() + 2);
}

void DragWindow::show()
{
	setWindowPos(true);
	QWidget::show();
}

void DragWindow::move(int deltaX, int deltaY)
{
	offset_ += QPoint(deltaX, deltaY);
	setWindowPos(isVisible());
}

void DragWindow::hide()
{
	setWindowPos(false);
	QWidget::hide();
}

struct DrawRowVisitor
{
	DrawRowVisitor(QPainter& painter) : painter_(painter) {}

	ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
	{
		if(row->pulledUp() && row->visible(tree))
			row->drawRow(painter_, tree, index);

		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	QPainter& painter_;
};

void DragWindow::drawRow(QPainter& p)
{
	QRect entireRowRect(0, 0, rect_.width() + 1, rect_.height() + 1);

	p.setBrush(tree_->palette().button());
	p.setPen(QPen(tree_->palette().color(QPalette::WindowText)));
	p.drawRect(entireRowRect);

	QPoint leftTop = row_->rect().topLeft();
	int offsetX = -leftTop.x() - tree_->tabSize() + 2;
	int offsetY = -leftTop.y() + 1;
	p.translate(offsetX, offsetY);
	int rowIndex = 0;
	if (row_->parent())
		rowIndex = row_->parent()->childIndex(row_);
	row_->drawRow(p, tree_, 0);
	DrawRowVisitor visitor(p);
	row_->scanChildren(visitor, tree_);
	p.translate(-offsetX, -offsetY);
}

void DragWindow::paintEvent(QPaintEvent* ev)
{
	QPainter p(this);

	drawRow(p);
}

// ---------------------------------------------------------------------------

class QPropertyTree::DragController
{
public:
	DragController(QPropertyTree* tree)
	: tree_(tree)
	, captured_(false)
	, dragging_(false)
	, before_(false)
	, row_(0)
	, clickedRow_(0)
	, window_(tree)
	, hoveredRow_(0)
	, destinationRow_(0)
	{
	}

	void beginDrag(PropertyRow* clickedRow, PropertyRow* draggedRow, QPoint pt)
	{
		row_ = draggedRow;
		clickedRow_ = clickedRow;
		startPoint_ = pt;
		lastPoint_ = pt;
		captured_ = true;
		dragging_ = false;
	}

	bool dragOn(QPoint screenPoint)
	{
		if (dragging_)
			window_.move(screenPoint.x() - lastPoint_.x(), screenPoint.y() - lastPoint_.y());

		bool needCapture = false;
		if(!dragging_ && (startPoint_ - screenPoint).manhattanLength() >= 5)
			if(row_->canBeDragged()){
				needCapture = true;
				QRect rect = row_->rect();
				rect = QRect(rect.topLeft() - tree_->offset_ + QPoint(tree_->tabSize(), 0), 
							 rect.bottomRight() - tree_->offset_);

				window_.set(tree_, row_, rect);
				window_.move(screenPoint.x() - startPoint_.x(), screenPoint.y() - startPoint_.y());
				window_.show();
				dragging_ = true;
			}

		if(dragging_){
			QPoint point = tree_->mapFromGlobal(screenPoint);
			trackRow(point);
		}
		lastPoint_ = screenPoint;
		return needCapture;
	}

	void interrupt()
	{
		captured_ = false;
		dragging_ = false;
		row_ = 0;
		window_.hide();
	}

	void trackRow(QPoint pt)
	{
		hoveredRow_ = 0;
		destinationRow_ = 0;

		QPoint point = pt;
		PropertyRow* row = tree_->rowByPoint(point);
		if(!row || !row_)
			return;

		row = row->nonPulledParent();
		if(!row->parent() || row->isChildOf(row_) || row == row_)
			return;

		float pos = (point.y() - row->rect().top()) / float(row->rect().height());
		if(row_->canBeDroppedOn(row->parent(), row, tree_)){
			if(pos < 0.25f){
				destinationRow_ = row->parent();
				hoveredRow_ = row;
				before_ = true;
				return;
			}
			if(pos > 0.75f){
				destinationRow_ = row->parent();
				hoveredRow_ = row;
				before_ = false;
				return;
			}
		}
		if(row_->canBeDroppedOn(row, 0, tree_))
			hoveredRow_ = destinationRow_ = row;
	}

	void drawUnder(QPainter& painter)
	{
		if(dragging_ && destinationRow_ == hoveredRow_ && hoveredRow_){
			QRect rowRect = hoveredRow_->rect();
			rowRect.setLeft(rowRect.left() + tree_->tabSize());
			QBrush brush(true ? tree_->palette().highlight() : tree_->palette().shadow());
			QColor brushColor = brush.color();
			QColor borderColor(brushColor.alpha() / 4, brushColor.red(), brushColor.green(), brushColor.blue());
			fillRoundRectangle(painter, brush, rowRect, borderColor, 6);
		}
	}

	void drawOver(QPainter& painter)
	{
		if(!dragging_)
			return;

		QRect rowRect = row_->rect();

		if(destinationRow_ != hoveredRow_ && hoveredRow_){
			const int tickSize = 4;
			QRect hoveredRect = hoveredRow_->rect();
			hoveredRect.setLeft(hoveredRect.left() + tree_->tabSize());

			if(!before_){ // previous
				QRect rect(hoveredRect.left() - 1 , hoveredRect.bottom() - 1, hoveredRect.width(), 2);
				QRect rectLeft(hoveredRect.left() - 1 , hoveredRect.bottom() - tickSize, 2, tickSize * 2);
				QRect rectRight(hoveredRect.right() - 1 , hoveredRect.bottom() - tickSize, 2, tickSize * 2);
				painter.fillRect(rect, tree_->palette().highlight());
				painter.fillRect(rectLeft, tree_->palette().highlight());
				painter.fillRect(rectRight, tree_->palette().highlight());
			}
			else{ // next
				QRect rect(hoveredRect.left() - 1 , hoveredRect.top() - 1, hoveredRect.width(), 2);
				QRect rectLeft(hoveredRect.left() - 1 , hoveredRect.top() - tickSize, 2, tickSize * 2);
				QRect rectRight(hoveredRect.right() - 1 , hoveredRect.top() - tickSize, 2, tickSize * 2);
				painter.fillRect(rect, tree_->palette().highlight());
				painter.fillRect(rectLeft, tree_->palette().highlight());
				painter.fillRect(rectRight, tree_->palette().highlight());
			}
		}
	}

	bool drop(QPoint screenPoint)
	{
		bool rowLayoutChanged = false;
		PropertyTreeModel* model = tree_->model();
		if(row_ && hoveredRow_){
			YASLI_ASSERT(destinationRow_);
			clickedRow_->setSelected(false);
			row_->dropInto(destinationRow_, destinationRow_ == hoveredRow_ ? 0 : hoveredRow_, tree_, before_);
			rowLayoutChanged = true;
		}

		captured_ = false;
		dragging_ = false;
		row_ = 0;
		window_.hide();
		hoveredRow_ = 0;
		destinationRow_ = 0;
		return rowLayoutChanged;
	}

	bool captured() const{ return captured_; }
	bool dragging() const{ return dragging_; }
	PropertyRow* draggedRow() { return row_; }
protected:
	DragWindow window_;
	QPropertyTree* tree_;
	PropertyRow* row_;
	PropertyRow* clickedRow_;
	PropertyRow* hoveredRow_;
	PropertyRow* destinationRow_;
	QPoint startPoint_;
	QPoint lastPoint_;
	bool captured_;
	bool dragging_;
	bool before_;
};

// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
QPropertyTree::QPropertyTree(QWidget* parent)
: QWidget(parent)
, sizeHint_(180, 180)
, model_(0)
, cursorX_(0)
, attachedPropertyTree_(0)
, autoRevert_(true)
, dragController_(new DragController(this))
, capturedRow_(0)
, leftBorder_(0)
, rightBorder_(0)
, filterMode_(false)

, applyTime_(0)
, revertTime_(0)
, updateHeightsTime_(0)
, paintTime_(0)
{
	scrollBar_ = new QScrollBar(Qt::Vertical, this);
	connect(scrollBar_, SIGNAL(valueChanged(int)), this, SLOT(onScroll(int)));

	(TreeConfig&)*this = defaultConfig;

	model_.reset(new PropertyTreeModel());
	model_->setExpandLevels(expandLevels_);
	model_->setUndoEnabled(undoEnabled_);
	model_->setFullUndo(fullUndo_);

	connect(model_.data(), SIGNAL(signalUpdated(const PropertyRows&)), this, SLOT(onModelUpdated(const PropertyRows&)));
	connect(model_.data(), SIGNAL(signalPushUndo(PropertyTreeOperator*, bool*)), this, SLOT(onModelPushUndo(PropertyTreeOperator*, bool*)));
	//model_->signalPushUndo().connect(this, &QPropertyTree::onModelPushUndo);

    filterEntry_.reset(new FilterEntry(this));
    QObject::connect(filterEntry_.data(), SIGNAL(textChanged(const QString&)), this, SLOT(onFilterChanged(const QString&)));
    filterEntry_->hide();

	mouseStillTimer_ = new QTimer();
	mouseStillTimer_->setSingleShot(true);
	connect(mouseStillTimer_, SIGNAL(timeout()), this, SLOT(onMouseStill()));

	DrawingCache::get()->initialize();
}
#pragma warning(pop)

QPropertyTree::~QPropertyTree()
{
	clearMenuHandlers();
	DrawingCache::get()->finalize();
}

bool QPropertyTree::onRowKeyDown(PropertyRow* row, const QKeyEvent* ev)
{
	PropertyTreeMenuHandler handler;
	handler.row = row;
	handler.tree = this;

	if(row->onKeyDown(this, ev))
		return true;

  switch(ev->key()){
		case Qt::Key_C:
			if (ev->modifiers() == Qt::CTRL)
				handler.onMenuCopy();
		return true;
		case Qt::Key_V:
			if (ev->modifiers() == Qt::CTRL)
				handler.onMenuPaste();
		return true;
		case Qt::Key_Z:
			if (ev->modifiers() == Qt::CTRL)
				if(model()->canUndo()){
					model()->undo();
					return true;
				}
		break;
	case Qt::Key_F2:
		if (ev->modifiers() == Qt::NoModifier) {
			if(selectedRow())
				selectedRow()->onActivate(this, true);
		}
		break;
	case Qt::Key_Menu:
		{
			if (ev->modifiers() == Qt::NoModifier) {
			QMenu menu(this);

			if(onContextMenu(row, menu)){
				QRect rect(row->rect());
				QPoint pt = _toScreen(QPoint(rect.left() + rect.height(), rect.bottom()));
				menu.exec(pt);
			}
			return true;
		}
		break;
	}
	}

	PropertyRow* focusedRow = model()->focusedRow();
	if(!focusedRow)
		return false;
	PropertyRow* parentRow = focusedRow->nonPulledParent();
	int x = parentRow->horizontalIndex(this, focusedRow);
	int y = model()->root()->verticalIndex(this, parentRow);
	PropertyRow* selectedRow = 0;
	switch(ev->key()){
	case Qt::Key_Up:
		if (filterMode_ && y == 0) {
			setFilterMode(true);
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, --y);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case Qt::Key_Down:
	if (filterMode_ && filterEntry_->hasFocus()) {
		setFocus();
	}
	else {
		selectedRow = model()->root()->rowByVerticalIndex(this, ++y);
		if (selectedRow)
			selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
	}
		break;
	case Qt::Key_Left:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = --x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && parentRow->expanded()){
			expandRow(parentRow, false);
			//model()->requestUpdate();
			selectedRow = model()->focusedRow();
		}
		break;
	case Qt::Key_Right:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = ++x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && !parentRow->expanded()){
			expandRow(parentRow, true);
			//model()->requestUpdate();
			selectedRow = model()->focusedRow();
		}
		break;
	case Qt::Key_Home:
		if (ev->modifiers() == Qt::CTRL) {
			selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MIN);
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, 0);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case Qt::Key_End:
		if (ev->modifiers() == Qt::CTRL) {
			selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MAX);
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, INT_MAX);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case Qt::Key_Space:
		if (filterWhenType_)
			break;
	case Qt::Key_Return:
		if(focusedRow->canBeToggled(this))
			expandRow(focusedRow, !focusedRow->expanded());
		else
			focusedRow->onActivate(this, false);
		break;
	}
	if(selectedRow){
		onRowSelected(selectedRow, false, false);	
		return true;
	}
	return false;
}

bool QPropertyTree::onRowLMBDown(PropertyRow* row, const QRect& rowRect, QPoint point, bool controlPressed)
{
	pressPoint_ = point;
	row = model()->root()->hit(this, point);
	if(row){
		if(!row->isRoot() && row->plusRect().contains(point) && toggleRow(row))
			return true;
		PropertyRow* rowToSelect = row;
		while (rowToSelect && !rowToSelect->isSelectable())
			rowToSelect = rowToSelect->parent();
		if (rowToSelect)
			onRowSelected(rowToSelect, multiSelectable() && controlPressed, true);	
	}

	PropertyTreeModel::UpdateLock lock = model()->lockUpdate();
	row = model()->root()->hit(this, point);
	if(row && !row->isRoot()){
		bool changed = false;
		bool capture = row->onMouseDown(this, point, changed);
		if(!changed && !widget_){ // FIXME: осмысленный метод для проверки
			if(capture)
				return true;
			else if(row->widgetRect().contains(point)){
				if(row->widgetPlacement() != PropertyRow::WIDGET_ICON)
					interruptDrag();
				row->onActivate(this, false);
				return false;
			}
		}
	}
	return false;
}

void QPropertyTree::onRowLMBUp(PropertyRow* row, const QRect& rowRect, QPoint point)
{
	row->onMouseUp(this, point);

	if ((pressPoint_ - point).manhattanLength() < 1 && row->widgetRect().contains(point)) {
		row->onActivateRelease(this);
	}
}

void QPropertyTree::onRowRMBDown(PropertyRow* row, const QRect& rowRect, QPoint point)
{
	SharedPtr<PropertyRow> handle = row;
	PropertyRow* menuRow = 0;

	if (row->isSelectable()){
		menuRow = row;
	}
	else{
		if (row->parent() && row->parent()->isSelectable())
			menuRow = row->parent();
	}

	if (menuRow) {
		onRowSelected(menuRow, false, true);	
		QMenu menu(this);
		clearMenuHandlers();
		if(onContextMenu(menuRow, menu))
			menu.exec(point);
	}
}

void QPropertyTree::expandParents(PropertyRow* row)
{
	bool hasChanges = false;
	typedef std::vector<PropertyRow*> Parents;
	Parents parents;
	PropertyRow* p = row->nonPulledParent()->parent();
	while(p){
		parents.push_back(p);
		p = p->parent();
	}
	Parents::iterator it;
	for(it = parents.begin(); it != parents.end(); ++it) {
		PropertyRow* row = *it;
		if (!row->expanded()) {
			row->_setExpanded(true);
			hasChanges = true;
		}
	}
	if (hasChanges)
		updateHeights();
}

void QPropertyTree::expandAll(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		for (PropertyRows::iterator it = root->begin(); it != root->end(); ++it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, true);
		}
	}
	else
		root->setExpandedRecursive(this, true);
	updateHeights();
}

void QPropertyTree::collapseAll(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		for (PropertyRows::iterator it = root->begin(); it != root->end(); ++it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, false);
		}
	}
	else{
		root->setExpandedRecursive(this, false);
		PropertyRow* row = model()->focusedRow();
		while(row){
			if(root == row){
				model()->selectRow(row, true);
				break;
			}
			row = row->parent();
		}
	}

	update();
}


void QPropertyTree::expandRow(PropertyRow* row, bool expanded, bool updateHeights)
{
	bool hasChanges = false;
	if (row->expanded() != expanded) {
		row->_setExpanded(expanded);
		hasChanges = true;
	}

	for (PropertyRow* r = row; r != 0; r = r->parent())
		r->setLayoutChanged();

    if(!row->expanded()){
		PropertyRow* f = model()->focusedRow();
		while(f){
			if(row == f){
				model()->selectRow(row, true);
				break;
			}
			f = f->parent();
		}
	}

	if (hasChanges && updateHeights)
		this->updateHeights();
}

void QPropertyTree::interruptDrag()
{
	dragController_->interrupt();
}

void QPropertyTree::updateHeights()
{
	QElapsedTimer timer;
	timer.start();

	model()->root()->updateLabel(this, 0);

	QRect widgetRect = this->rect();

	int scrollBarW = 16;
	int lb = compact_ ? 0 : 4;
	int rb = widgetRect.right() - lb - scrollBarW;
	bool force = lb != leftBorder_ || rb != rightBorder_;
	leftBorder_ = lb;
	rightBorder_ = rb;
	model()->root()->calculateMinimalSize(this, leftBorder_, force, 0, 0);

	int padding = compact_ ? 4 : 6;

	int totalHeight = 0;
	model()->root()->adjustVerticalPosition(this, totalHeight);
	size_.setY(totalHeight);

	bool hasScrollBar = updateScrollBar();

	area_.setLeft(widgetRect.left() + 2);
	area_.setRight(widgetRect.right() - 2 - scrollBarW);
	area_.setTop(widgetRect.top() + 2);
	area_.setBottom(widgetRect.bottom() - 2);
	size_.setX(area_.width());

	if (filterMode_)
	{
		int filterAreaHeight = filterEntry_ ? filterEntry_->height() : 0;
		area_.setTop(area_.top() + filterAreaHeight + 2 + 2);
	}

	_arrangeChildren();

	update();
	updateHeightsTime_ = timer.elapsed();
}

bool QPropertyTree::updateScrollBar()
{
	int pageSize = rect().height();
	offset_.setX(max(0, min(offset_.x(), max(0, size_.x() - area_.right() - 1))));
	offset_.setY(max(0, min(offset_.y(), max(0, size_.y() - pageSize))));

	if (pageSize < size_.y())
	{
		scrollBar_->setRange(0, size_.y() - pageSize);
		scrollBar_->setSliderPosition(offset_.y());
		scrollBar_->setPageStep(pageSize);
		scrollBar_->show();
		scrollBar_->move(rect().right() - scrollBar_->width(), 0);
		scrollBar_->resize(scrollBar_->width(), height());
		return true;
	}
	else
	{
		scrollBar_->hide();
		return false;
	}
}

QPoint QPropertyTree::treeSize() const
{
	return size_ + (compact() ? QPoint(0,0) : QPoint(8, 8));
}

void QPropertyTree::onScroll(int pos)
{
	offset_.setY(scrollBar_->sliderPosition());
	_arrangeChildren();
	repaint();
}

void QPropertyTree::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_STATE)){
		model()->serialize(ar, this);

		if(ar.isInput()){
			ensureVisible(model()->focusedRow());
			updateAttachedPropertyTree();
			update();
			signalSelected();
		}
	}
}

void QPropertyTree::ensureVisible(PropertyRow* row, bool update)
{
	if (row == 0)
		return;
	if(row->isRoot())
		return;

	expandParents(row);

	QRect rect = row->rect();
	if(rect.top() < area_.top() + offset_.y()){
		offset_.setY(max(0, rect.top() - area_.top()));
	}
	else if(rect.bottom() > area_.bottom() + offset_.y()){
		offset_.setY(max(0, rect.bottom() - area_.bottom()));
	}
	if(update)
		this->update();
}

void QPropertyTree::onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos)
{
	if(!row->isRoot())
		model()->selectRow(row, !(addSelection && row->selected() && model()->selection().size() > 1), !addSelection);
	ensureVisible(row);
	if(adjustCursorPos)
		cursorX_ = row->nonPulledParent()->horizontalIndex(this, row);
	updateAttachedPropertyTree();
	signalSelected();
}

void QPropertyTree::attach(const yasli::Serializers& serializers)
{
	attached_.clear();
	for (size_t i = 0; i < serializers.size(); ++i)
		attached_.push_back(yasli::Object(serializers[i]));

	model_->setRootObject(attached_.empty() ? Object() : attached_[0]);
	revert();
}

void QPropertyTree::attach(const yasli::Serializer& serializer)
{
	attach(yasli::Object(serializer));
}

void QPropertyTree::attach(const yasli::Object& object)
{
	attached_.clear();
	attached_.push_back(object);

	model_->setRootObject(object);
	revert();

	updateHeights();
}

void QPropertyTree::detach()
{
	if(widget_)
		widget_.reset();
	attached_.clear();
	model()->root()->clear();
	update();
}

int QPropertyTree::revertObjects(vector<void*> objectAddresses)
{
	int result = 0;
	for (size_t i = 0; i < objectAddresses.size(); ++i) {
		if (revertObject(objectAddresses[i]))
			++result;
	}
	return result;
}

bool QPropertyTree::revertObject(void* objectAddress)
{
	ModelObjectReferences& refs = model_->objectReferences();
	ModelObjectReferences::iterator it = refs.find(objectAddress);
	if (it == refs.end())
		return false;
	it->second.needUpdate = true;
	revertChanged(false);
	return true;
}

void QPropertyTree::revertChanged(bool enforce)
{	
	printf("revertChanges()\n");
	QElapsedTimer timer;
	timer.start();

	bool enforceNext = enforce;
	vector<ModelObjectReference> objectsToUpdate;

	while (true) {
		ModelObjectReferences& refs = model_->objectReferences();
		for (ModelObjectReferences::iterator it = refs.begin(); it != refs.end(); ++it) {
			ModelObjectReference& ref = it->second;
			if (enforceNext || ref.needUpdate) {
				ref.needUpdate = false;
				objectsToUpdate.push_back(ref);
			}
		}
		enforceNext = false;

		if (objectsToUpdate.empty())
			break;

		for (size_t i = 0; i < objectsToUpdate.size(); ++i) {
			ModelObjectReference& ref = objectsToUpdate[i];
			PropertyOArchive oa(model_.data(), ref.row);
			oa.setFilter(filter_);
			Object obj = ref.row->object();
			if (obj.isSet())
				obj(oa);
			else
				ref.row->clear();
			ref.row->setLabelChanged();
		}
		objectsToUpdate.clear();
	}

	if (model()->root())
		model_->root()->updateLabel(this, 0);
	if (filterMode_)
		onFilterChanged(QString());

	revertTime_ = timer.elapsed();
	updateHeights();
}

void QPropertyTree::revert()
{

	interruptDrag();
	widget_.reset();

	if (!attached_.empty()) {
		revertChanged(true);
	}
	else
		model_->clear();

	if (filterMode_) {
		if (model_->root())
			model_->root()->updateLabel(this, 0);
        onFilterChanged(QString());
	}


	update();
	updateAttachedPropertyTree();

	signalReverted();
}

void QPropertyTree::apply()
{
	applyChanged(true);
}

void QPropertyTree::applyChanged(bool enforce)
{
	//printf("applyChanges()\n");
	QElapsedTimer timer;
	timer.start();
	ModelObjectReferences& refs = model_->objectReferences();
	ModelObjectReferences::iterator it;

	for (it = refs.begin(); it != refs.end(); ++it) {
		ModelObjectReference& ref = it->second;
		if (!enforce && !ref.needApply)
			continue;

		Object obj = ref.row->object();
		PropertyIArchive ia(model_.data(), ref.row);
		Archive::Context<QPropertyTree> treeContext(ia, this);
		ia.setFilter(filter_);
		obj(ia);
		ref.needApply = false;
		signalObjectChanged(obj);
	}
	signalChanged();
	applyTime_ = timer.elapsed();

	updateHeights();
}

bool QPropertyTree::spawnWidget(PropertyRow* row, bool ignoreReadOnly)
{
	if(!widget_ || widget_->row() != row){
		interruptDrag();
		setWidget(0);
		PropertyRowWidget* newWidget = 0;
		if (ignoreReadOnly && row->userReadOnlyRecurse() || !row->userReadOnly())
			newWidget = row->createWidget(this);
		setWidget(newWidget);
		return newWidget != 0;
	}
	return false;
}

bool QPropertyTree::activateRow(PropertyRow* row)
{
	interruptDrag();
	return row->onActivate(this, false);
}

void QPropertyTree::addMenuHandler(PropertyRowMenuHandler* handler)
{
	menuHandlers_.push_back(handler);
}

void QPropertyTree::clearMenuHandlers()
{
	for (size_t i = 0; i < menuHandlers_.size(); ++i)
	{
		PropertyRowMenuHandler* handler = menuHandlers_[i];
		delete handler;
	}
	menuHandlers_.clear();
}

static yasli::string quoteIfNeeded(const char* str)
{
	if (strchr(str, ' ') != 0) {
		yasli::string result;
		result = "\"";
		result += str;
		result += "\"";
		return result;
	}
	else {
		return yasli::string(str);
	}
}

bool QPropertyTree::onContextMenu(PropertyRow* r, QMenu& menu)
{
	SharedPtr<PropertyRow> row(r);
	PropertyTreeMenuHandler* handler = new PropertyTreeMenuHandler();
	addMenuHandler(handler);
	handler->tree = this;
	handler->row = row;

	PropertyRow::iterator it;
	for(it = row->begin(); it != row->end(); ++it){
		PropertyRow* child = *it;
		if(child->isContainer() && child->pulledUp())
			child->onContextMenu(menu, this);
	}
	row->onContextMenu(menu, this);
	if(undoEnabled_){
		if(!menu.isEmpty())
			menu.addSeparator();
		QAction* undo = menu.addAction("Undo");
		undo->setEnabled(model()->canUndo());
		undo->setShortcut(QKeySequence("Ctrl+Z"));
		connect(undo, SIGNAL(triggered()), handler, SLOT(onMenuUndo()));
	}
	if(!menu.isEmpty())
		menu.addSeparator();

	QAction* copy = menu.addAction("Copy");
	copy->setShortcut(QKeySequence("Ctrl+C"));
	connect(copy, SIGNAL(triggered()), handler, SLOT(onMenuCopy()));

	if(!row->userReadOnly()){
		QAction* paste = menu.addAction("Paste");
		paste->setShortcut(QKeySequence("Ctrl+V"));
		connect(paste, SIGNAL(triggered()), handler, SLOT(onMenuPaste()));
		paste->setEnabled(canBePasted(row));
	}

	menu.addSeparator();

	QMenu* filter = menu.addMenu("Filter by");
	{
		yasli::string nameFilter = quoteIfNeeded(row->labelUndecorated());
		handler->filterName = nameFilter;
		connect(filter->addAction((yasli::string("Name:\t") + nameFilter).c_str()), SIGNAL(triggered()), handler, SLOT(onMenuFilterByName()));

		yasli::string valueFilter = "=";
		valueFilter += quoteIfNeeded(row->valueAsString().c_str());
		handler->filterValue = valueFilter;
		connect(filter->addAction((yasli::string("Value:\t") + valueFilter).c_str()), SIGNAL(triggered()), handler, SLOT(onMenuFilterByValue()));

		yasli::string typeFilter = ":";
		typeFilter += quoteIfNeeded(row->typeNameForFilter());
		handler->filterType = typeFilter;
		connect(filter->addAction((yasli::string("Type:\t") + typeFilter).c_str()), SIGNAL(triggered()), handler, SLOT(onMenuFilterByType()));
	}

#if 0
	menu.addSeparator();
	menu.addAction(TRANSLATE("Decompose"), row).connect(this, &QPropertyTree::onRowMenuDecompose);
#endif
	return true;
}

void QPropertyTree::onRowMouseMove(PropertyRow* row, const QRect& rowRect, QPoint point)
{
	PropertyDragEvent e;
	e.tree = this;
	e.pos = point;
	e.start = pressPoint_;
	row->onMouseDrag(e);
	update();
}


bool QPropertyTree::canBePasted(PropertyRow* destination)
{
	SharedPtr<PropertyRow> source;
	if (!propertyRowFromClipboard(source, &constStrings_))
		return false;

	if (!smartPaste(destination, source, model_.data(), true))
		return false;
	return true;
}

bool QPropertyTree::canBePasted(const char* destinationType)
{
	SharedPtr<PropertyRow> source;
	if (!propertyRowFromClipboard(source, &constStrings_))
		return false;

	bool result = strcmp(source->typeName(), destinationType) == 0;
	return result;
}

struct DecomposeProxy
{
	DecomposeProxy(SharedPtr<PropertyRow>& row) : row(row) {}
	
	void serialize(yasli::Archive& ar)
	{
		ar(row, "row", "Row");
	}

	SharedPtr<PropertyRow>& row;
};

void QPropertyTree::onRowMenuDecompose(PropertyRow* row)
{
  // SharedPtr<PropertyRow> clonedRow = row->clone();
  // DecomposeProxy proxy(clonedRow);
  // edit(SStruct(proxy), 0, IMMEDIATE_UPDATE, this);
}

void QPropertyTree::onModelUpdated(const PropertyRows& rows)
{
	if(widget_)
		widget_.reset();

	if(immediateUpdate_){

		ModelObjectReferences& refs = model_->objectReferences();

		size_t count = rows.size();
		for (size_t i = 0; i < count; ++i) {
			void *objectAddress = 0;
			PropertyRow* row = rows[i];
			if (row->isObject())
				objectAddress = static_cast<PropertyRowObject*>(row)->object().address();
			else
				objectAddress = row->serializer().pointer();
			if (!objectAddress)
				continue;

			ModelObjectReferences::iterator it = refs.find(objectAddress);
			if (it != refs.end()) {
				ModelObjectReference& ref = it->second;
				ref.needUpdate = true;
				ref.needApply = true;
			}
		}

		applyChanged(false);

		if(autoRevert_)
			revertChanged(false);
	}

	setFocus();

	updateHeights();

	updateAttachedPropertyTree();
	if(!immediateUpdate_)
		onSignalChanged();
}

void QPropertyTree::onModelPushUndo(PropertyTreeOperator* op, bool* handled)
{
	signalPushUndo();
}

void QPropertyTree::setWidget(PropertyRowWidget* widget)
{
	if(widget_){
		//oldWidget->commit();
	}
	widget_.reset(widget);
	model()->dismissUpdate();
	if(widget_){
		YASLI_ASSERT(widget_->actualWidget());
		if(widget_->actualWidget())
			widget_->actualWidget()->setParent(this);
		_arrangeChildren();
	}
}

bool QPropertyTree::hasFocusOrInplaceHasFocus() const
{
	if (hasFocus())
		return true;

	if (widget_ && widget_->actualWidget() && widget_->actualWidget()->hasFocus())
		return true;

	return false;
}

void QPropertyTree::setFilterMode(bool inFilterMode)
{
    bool changed = filterMode_ != inFilterMode;
    filterMode_ = inFilterMode;
    
	if (filterMode_)
	{
        filterEntry_->show();
		filterEntry_->setFocus();
        filterEntry_->selectAll();
	}
    else
        filterEntry_->hide();

    if (changed)
    {
        onFilterChanged(QString());
        updateArea();
    }
}

void QPropertyTree::startFilter(const char* filter)
{
	setFilterMode(true);
	filterEntry_->setText(filter);
    onFilterChanged(filter);
}




void QPropertyTree::_arrangeChildren()
{
	if(widget_){
		PropertyRow* row = widget_->row();
		if(row->visible(this)){
			QWidget* w = widget_->actualWidget();
			YASLI_ASSERT(w);
			if(w){
				QRect rect = row->widgetRect();
				rect = QRect(rect.topLeft() - offset_ + area_.topLeft(), 
										 rect.bottomRight() - offset_ + area_.topLeft());
				w->move(rect.topLeft());
				w->resize(rect.size());
				if(!w->isVisible()){
					w->show();
					w->setFocus();
				}
			}
			else{
				//YASLI_ASSERT(w);
			}
		}
		else{
			widget_.reset();
		}
	}

	if (filterEntry_) {
		QSize size = rect().size();
		const int padding = 2;
        QRect pos(padding, padding, size.width() - padding * 2, filterEntry_->height());
		filterEntry_->move(pos.topLeft());
		filterEntry_->resize(pos.size() - QSize(scrollBar_ ? scrollBar_->width() : 0, 0));
	}
}



void QPropertyTree::setExpandLevels(int levels)
{
	expandLevels_ = levels;
    model()->setExpandLevels(levels);
}

PropertyRow* QPropertyTree::selectedRow()
{
    const PropertyTreeModel::Selection &sel = model()->selection();
    if(sel.empty())
        return 0;
    return model()->rowFromPath(sel.front());
}

bool QPropertyTree::getSelectedObject(Object* object)
{
	const PropertyTreeModel::Selection &sel = model()->selection();
	if(sel.empty())
		return 0;
	PropertyRow* row = model()->rowFromPath(sel.front());
	while (row && !row->isObject())
		row = row->parent();
	if (!row)
		return false;

	PropertyRowObject* obj = static_cast<PropertyRowObject*>(row);
	*object = obj->object();
	return true;
}

QPoint QPropertyTree::_toScreen(QPoint point) const
{
	QPoint pt ( point.x() - offset_.x() + area_.left(), 
				point.y() - offset_.y() + area_.top() );

	return mapToGlobal(pt);
}

bool QPropertyTree::selectByAddress(void* addr, bool keepSelectionIfChildSelected)
{
	if (model()->root()) {
		PropertyRow* row = model()->root()->findByAddress(addr);

		bool keepSelection = false;
		if (keepSelectionIfChildSelected && row && !model()->selection().empty()) {
			keepSelection = true;
			TreeSelection::const_iterator it;
			for(it = model()->selection().begin(); it != model()->selection().end(); ++it){
				PropertyRow* selectedRow = model()->rowFromPath(*it);
				if (!selectedRow)
					continue;
				if (!selectedRow->isChildOf(row)){
					keepSelection = false;
					break;
				}
			}
		}

		if (!keepSelection) {
			TreeSelection sel;
			if(row)
				sel.push_back(model()->pathFromRow(row));
			if (model()->selection() != sel) {
				model()->setSelection(sel);
				if (row)
					ensureVisible(row);
				repaint();
				return true;
			}
		}
	}
	return false;
}

void QPropertyTree::setUndoEnabled(bool enabled, bool full)
{
	undoEnabled_ = enabled; fullUndo_ = full;
    model()->setUndoEnabled(enabled);
    model()->setFullUndo(full);
}

void QPropertyTree::attachPropertyTree(QPropertyTree* propertyTree) 
{ 
	// if(attachedPropertyTree_)
	//	attachedPropertyTree_->signalChanged().disconnect(this);
	//	attachedPropertyTree_ = propertyTree; 
	//	attachedPropertyTree_->signalChanged().connect(this, &QPropertyTree::revert);
	//	updateAttachedPropertyTree(); 
}

void QPropertyTree::getSelectionSerializers(yasli::Serializers* serializers)
{
		TreeSelection::const_iterator i;
		for(i = model()->selection().begin(); i != model()->selection().end(); ++i){
			PropertyRow* row = model()->rowFromPath(*i);
			if (!row)
				continue;

			Serializer ser = row->serializer();

			if (!ser) {
				while(row && (row->pulledUp() || row->pulledBefore())) {
					row = row->parent();
				}
				ser = row->serializer();
			}
			
			if (ser)
				serializers->push_back(ser);
		}
}

void QPropertyTree::updateAttachedPropertyTree()
{
	if (!attachedPropertyTree_)
		return;

	Object obj;
	if (getSelectedObject(&obj))
		attachedPropertyTree_->attach(obj);
	else
		attachedPropertyTree_->detach();
}

struct FilterVisitor
{
	const QPropertyTree::RowFilter& filter_;

	FilterVisitor(const QPropertyTree::RowFilter& filter) 
    : filter_(filter)
    {
    }

	static void markChildrenAsBelonging(PropertyRow* row, bool belongs)
	{
		int count = int(row->count());
		for (int i = 0; i < count; ++i)
		{
			PropertyRow* child = row->childByIndex(i);
			child->setBelongsToFilteredRow(belongs);

			markChildrenAsBelonging(child, belongs);
		}
	}
	
	static bool hasMatchingChildren(PropertyRow* row)
	{
		int numChildren = (int)row->count();
		for (int i = 0; i < numChildren; ++i)
		{
			PropertyRow* child = row->childByIndex(i);
			if (!child)
				continue;
			if (child->matchFilter())
				return true;
			if (hasMatchingChildren(child))
				return true;
		}
		return false;
	}

	ScanResult operator()(PropertyRow* row, QPropertyTree* tree)
	{
		const char* label = row->labelUndecorated();
		bool matchFilter = filter_.match(label, filter_.NAME, 0, 0);
		if (matchFilter)
			matchFilter = filter_.match(row->valueAsString().c_str(), filter_.VALUE, 0, 0);
		if (matchFilter && filter_.typeRelevant(filter_.TYPE))
			matchFilter = filter_.match(row->typeNameForFilter(), filter_.TYPE, 0, 0);						   
		
		int numChildren = int(row->count());
		if (matchFilter) {
			if (row->pulledBefore() || row->pulledUp()) {
				// treat pulled rows as part of parent
				PropertyRow* parent = row->parent();
				parent->setMatchFilter(true);
				markChildrenAsBelonging(parent, true);
				parent->setBelongsToFilteredRow(false);
			}
			else {
				markChildrenAsBelonging(row, true);
				row->setBelongsToFilteredRow(false);
				row->setLayoutChanged();
				row->setLabelChanged();
			}
		}
		else {
			bool belongs = hasMatchingChildren(row);
			row->setBelongsToFilteredRow(belongs);
			if (belongs) {
				tree->expandRow(row, true, false);
				for (int i = 0; i < numChildren; ++i) {
					PropertyRow* child = row->childByIndex(i);
					if (child->pulledUp())
						child->setBelongsToFilteredRow(true);
				}
			}
			else {
				row->_setExpanded(false);
				row->setLayoutChanged();
			}
		}

		row->setMatchFilter(matchFilter);
		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	yasli::string labelStart_;
};



void QPropertyTree::RowFilter::parse(const char* filter)
{
	for (int i = 0; i < NUM_TYPES; ++i) {
		start[i].clear();
		substrings[i].clear();
		tillEnd[i] = false;
	}

	YASLI_ESCAPE(filter != 0, return);

	vector<char> filterBuf(filter, filter + strlen(filter) + 1);
	for (size_t i = 0; i < filterBuf.size(); ++i)
		filterBuf[i] = tolower(filterBuf[i]);

	const char* str = &filterBuf[0];

	Type type = NAME;
	while (true)
	{
		bool fromStart = true;
		while (*str == ' ' || *str == '*') {
			fromStart = false;
			++str;
		}

		const char* tokenStart = str;
		
		if (*str == '\"')
		{
			tokenStart;
			++str;
			while(*str != '\0' && *str != '\"')
				++str;
		}
		else
		{
			while (*str != '\0' && *str != ' ' && *str != '*' && *str != '=' && *str != ':')
					++str;
		}
		if (str != tokenStart) {
			if (*tokenStart == '\"' && *str == '\"') {
				start[type].assign(tokenStart + 1, str);
				tillEnd[type] = true;
				++str;
			}
			else
			{
				if (fromStart)
					start[type].assign(tokenStart, str);
				else
					substrings[type].push_back(yasli::string(tokenStart, str));
			}
		}

		if (*str == '=') {
			type = VALUE;
			++str;
		}
		else if(*str == ':') {
			type = TYPE;
			++str;
		}
		else if (*str == '\0')
			break;
	}
}

bool QPropertyTree::RowFilter::match(const char* textOriginal, Type type, size_t* matchStart, size_t* matchEnd) const
{
	YASLI_ESCAPE(textOriginal, return false);

	char* text;
	{
		size_t textLen = strlen(textOriginal);
        text = (char*)alloca((textLen + 1));
		memcpy(text, textOriginal, (textLen + 1));
		for (char* p = text; *p; ++p)
			*p = tolower(*p);
	}
	
	const yasli::string &start = this->start[type];
	if (tillEnd[type]){
		if (start == text) {
			if (matchStart)
				*matchStart = 0;
			if (matchEnd)
				*matchEnd = start.size();
			return true;
		}
		else
			return false;
	}

	const vector<yasli::string> &substrings = this->substrings[type];

	const char* startPos = text;

	if (matchStart)
		*matchStart = 0;
	if (matchEnd)
		*matchEnd = 0;
	if (!start.empty()) {
		if (strncmp(text, start.c_str(), start.size()) != 0){
            //_freea(text);
			return false;
		}
		if (matchEnd)
			*matchEnd = start.size();
		startPos += start.size();
	}

	size_t numSubstrings = substrings.size();
	for (size_t i = 0; i < numSubstrings; ++i) {
		const char* substr = strstr(startPos, substrings[i].c_str());
		if (!substr){
            //_freea(text);
			return false;
		}
		startPos += substrings[i].size();
		if (matchStart && i == 0 && start.empty()) {
			*matchStart = substr - text;
		}
		if (matchEnd)
			*matchEnd = substr - text + substrings[i].size();
	}
    //_freea(text);
	return true;
}

void QPropertyTree::onFilterChanged(const QString& text)
{
	QByteArray arr = filterEntry_->text().toLocal8Bit();
	const char* filterStr = filterMode_ ? arr.data() : "";
	rowFilter_.parse(filterStr);
	FilterVisitor visitor(rowFilter_);
	model()->root()->scanChildrenBottomUp(visitor, this);
	updateHeights();
}

void QPropertyTree::drawFilteredString(QPainter& p, const wchar_t* text, RowFilter::Type type, const QFont* font, const QRect& rect, const QColor& textColor, bool pathEllipsis, bool center) const
{
	int textLen = (int)wcslen(text);

    if (textLen == 0)
        return;

	QRect textRect = rect;
	if (filterMode_) {
        size_t hiStart = 0;
        size_t hiEnd = 0;
        yasli::string textStr(fromWideChar(text));
        if (rowFilter_.match(textStr.c_str(), type, &hiStart, &hiEnd) && hiStart != hiEnd) {
            QRectF boxFull;
            QRectF boxStart;
            QRectF boxEnd;

            QString str(textStr.c_str());
            QFontMetrics fm(*font);
            boxFull = fm.boundingRect(textRect, Qt::AlignLeft | Qt::AlignVCenter, str);

            if (hiStart > 0)
                boxStart = fm.boundingRect(textRect, Qt::AlignLeft | Qt::AlignVCenter, str.left(hiStart));
            else {
                boxStart = fm.boundingRect(textRect, Qt::AlignLeft | Qt::AlignVCenter, str);
                boxStart.setWidth(0.0f);
            }
            boxEnd = fm.boundingRect(textRect, Qt::AlignLeft | Qt::AlignVCenter, str.left(hiEnd));

            QColor highlightColor, highlightBorderColor;
            {
                highlightColor = palette().color(QPalette::Highlight);
                int h, s, v;
                highlightColor.getHsv(&h, &s, &v);
                h -= 175;
                if (h < 0)
                    h += 360;
                highlightColor.setHsv(h, min(255, int(s * 1.33f)), v, 255);
                highlightBorderColor.setHsv(h, s * 0.5f, v, 255);
            }

            int left = int(boxFull.left() + boxStart.width()) - 1;
            int top = int(boxFull.top());
            int right = int(boxFull.left() + boxEnd.width());
            int bottom = int(boxFull.top() + boxEnd.height());
            QRect highlightRect(left, top, right - left, bottom - top);
            QBrush br(highlightColor);
            p.setBrush(br);
            p.setPen(highlightBorderColor);
            bool oldAntialiasing = p.renderHints().testFlag(QPainter::Antialiasing);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.drawRoundedRect(highlightRect, 4.0, 4.0);
            p.setRenderHint(QPainter::Antialiasing, oldAntialiasing);
        }
	}
	
	QBrush textBrush(textColor);
	p.setBrush(textBrush);
	p.setPen(textColor);
	QFont previousFont = p.font();
	p.setFont(*font);
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString(fromWideChar(text).c_str()), 0);
	p.setFont(previousFont);
}

void QPropertyTree::_drawRowLabel(QPainter& p, const wchar_t* text, const QFont* font, const QRect& rect, const QColor& textColor) const
{
	drawFilteredString(p, text, RowFilter::NAME, font, rect, textColor, false, false);
}

void QPropertyTree::_drawRowValue(QPainter& p, const wchar_t* text, const QFont* font, const QRect& rect, const QColor& textColor, bool pathEllipsis, bool center) const
{
	drawFilteredString(p, text, RowFilter::VALUE, font, rect, textColor, pathEllipsis, center);
}

struct DrawVisitor
{
	DrawVisitor(QPainter& painter, const QRect& area, int scrollOffset)
		: area_(area)
		, painter_(painter)
		, offset_(0)
		, scrollOffset_(scrollOffset)
		, lastParent_(0)
	{}

	ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
	{
		if(row->visible(tree) && (row->parent()->expanded() && !lastParent_ || row->pulledUp())){
			if(row->rect().top() > scrollOffset_ + area_.height())
				lastParent_ = row->parent();

			if(row->rect().bottom() > scrollOffset_)
				row->drawRow(painter_, tree, index);

			return SCAN_CHILDREN_SIBLINGS;
		}
		else
			return SCAN_SIBLINGS;
	}

protected:
	QPainter& painter_;
	QRect area_;
	int offset_;
	int scrollOffset_;
	PropertyRow* lastParent_;
};

QSize QPropertyTree::sizeHint() const
{
	return sizeHint_;
}

void QPropertyTree::paintEvent(QPaintEvent* ev)
{
	QElapsedTimer timer;
	timer.start();
	QPainter painter(this);
	QRect clientRect = this->rect();

	int clientWidth = clientRect.width();
	int clientHeight = clientRect.height();
	painter.fillRect(clientRect, palette().window());

	if (filterMode_)
	{
		// Win32::AutoSelector font(dc, Win32::defaultBoldFont());

		// SetBkMode(dc, TRANSPARENT);
		// const wchar_t filterStr[] = L"Filter:";
		// Vect2 size = Win32::calculateTextSize(handle(), Win32::defaultBoldFont(), filterStr);
		// int right = tree_->filterEntry_->_position().left();
		// ExtTextOutW(dc, right - size.x - 6, 6, 0, 0, filterStr, ARRAY_LEN(filterStr) - 1, 0);
	}

	//painter.setClipRect(area_);

	painter.translate(-offset_.x(), -offset_.y());

	if(dragController_->captured())
	 	dragController_->drawUnder(painter);

	painter.translate(area_.left(), area_.top());

    if (model()->root()) {
        DrawVisitor op(painter, area_, offset_.y());
        model()->root()->scanChildren(op, this);
    }

	painter.translate(-area_.left(), -area_.top());
	painter.translate(offset_.x(), offset_.y());

	//painter.setClipRect(rect());

	if (size_.y() > clientHeight)
	{
 	  const int shadowHeight = 10;
		QColor color1(0, 0, 0, 0);
		QColor color2(0, 0, 0, 96);

		QRect upperRect(rect().left(), rect().top(), area_.width(), shadowHeight);
		QLinearGradient upperGradient(upperRect.left(), upperRect.top(), upperRect.left(), upperRect.bottom());
		upperGradient.setColorAt(0.0f, color2);
		upperGradient.setColorAt(1.0f, color1);
		QBrush upperBrush(upperGradient);
		painter.fillRect(upperRect, upperBrush);

		QRect lowerRect(rect().left(), rect().bottom() - shadowHeight / 2, rect().width(), shadowHeight / 2 + 1);
		QLinearGradient lowerGradient(lowerRect.left(), lowerRect.top(), lowerRect.left(), lowerRect.bottom());		
		lowerGradient.setColorAt(0.0f, color1);
		lowerGradient.setColorAt(1.0f, color2);
		QBrush lowerBrush(lowerGradient);
		painter.fillRect(lowerRect, lowerGradient);
	}
	
	if (dragController_->captured()) {
	 	painter.translate(-offset_);
	 	dragController_->drawOver(painter);
	 	painter.translate(offset_);
	}
	else{
	// 	if(model()->focusedRow() != 0 && model()->focusedRow()->isRoot() && tree_->hasFocus()){
	// 		clientRect.left += 2; clientRect.top += 2;
	// 		clientRect.right -= 2; clientRect.bottom -= 2;
	// 		DrawFocusRect(dc, &clientRect);
	// 	}
	}
	paintTime_ = timer.elapsed();
}

QPropertyTree::HitTest QPropertyTree::hitTest(PropertyRow* row, const QPoint& pointInWindowSpace, const QRect& rowRect)
{
	QPoint point = pointToRootSpace(pointInWindowSpace);

	if(!row->hasVisibleChildren(this) && row->plusRect().contains(point))
		return TREE_HIT_PLUS;

	if(row->textRect().contains(point))
		return TREE_HIT_TEXT;

	if(rowRect.contains(point))
		return TREE_HIT_ROW;

	return TREE_HIT_NONE;

}

PropertyRow* QPropertyTree::rowByPoint(const QPoint& pt)
{
	if (!model_->root())
		return 0;
	if (!area_.contains(pt))
		return 0;
  return model_->root()->hit(this, pointToRootSpace(pt));
}

QPoint QPropertyTree::pointToRootSpace(const QPoint& point) const
{
	return QPoint(point.x() + offset_.x() - area_.left(), point.y() + offset_.y() - area_.top());
}

void QPropertyTree::moveEvent(QMoveEvent* ev)
{
	QWidget::moveEvent(ev);
}

void QPropertyTree::resizeEvent(QResizeEvent* ev)
{
	QWidget::resizeEvent(ev);

	updateHeights();
}

void QPropertyTree::mousePressEvent(QMouseEvent* ev)
{
	//QWidget::mousePressEvent(ev);
	setFocus(Qt::MouseFocusReason);

	if (ev->button() == Qt::LeftButton)
	{
		updateArea();
		int x = ev->x();
		int y = ev->y();

		PropertyRow* row = rowByPoint(ev->pos());
		if(row && !row->isSelectable())
			row = row->parent();
		if(row){
			if(onRowLMBDown(row, row->rect(), pointToRootSpace(ev->pos()), ev->modifiers().testFlag(Qt::ControlModifier)))
				capturedRow_ = row;
			else{
				row = rowByPoint(ev->pos());
				PropertyRow* draggedRow = row;
				while (draggedRow && (!draggedRow->isSelectable() || draggedRow->pulledUp() || draggedRow->pulledBefore()))
					draggedRow = draggedRow->parent();
				if(draggedRow && !draggedRow->userReadOnly() && !widget_){
					dragController_->beginDrag(row, draggedRow, ev->globalPos());
				}
			}
		}
		update();
	}
	else if (ev->button() == Qt::RightButton)
	{
		QPoint point = ev->pos();
		PropertyRow* row = rowByPoint(point);
		if(row){
			model()->setFocusedRow(row);
			update();

			onRowRMBDown(row, row->rect(), _toScreen(pointToRootSpace(point)));
		}
		else{
			QRect rect = this->rect();
			onRowRMBDown(model()->root(), rect, _toScreen(pointToRootSpace(point)));
		}
	}
	else if (ev->button() == Qt::MiddleButton)
	{
		QPoint point = ev->pos();
		PropertyRow* row = rowByPoint(point);
		if(row){
			switch(hitTest(row, point, row->rect())){
			case TREE_HIT_PLUS:
			break;
			case TREE_HIT_NONE:
			default:
			model()->setFocusedRow(row);
			update();
			break;
			}

		}
	}
}

void QPropertyTree::mouseReleaseEvent(QMouseEvent* ev)
{
	QWidget::mouseReleaseEvent(ev);

	if (ev->button() == Qt::LeftButton)
	{
		 if(dragController_->captured()){
		 	if (dragController_->drop(QCursor::pos()))
				updateHeights();
			else
				update();
		}
		QPoint point = ev->pos();
		PropertyRow* row = rowByPoint(point);
		if(row){
			switch(hitTest(row, point, row->rect())){
			case TREE_HIT_ROW:
				;
			//if(hoveredRow_)
			//	update();
			}			
		}
		if(capturedRow_){
			QRect rowRect = capturedRow_->rect();
			onRowLMBUp(capturedRow_, rowRect, pointToRootSpace(ev->pos()));
			mouseStillTimer_->stop();
			capturedRow_ = 0;
			update();
		}
	}
	else if (ev->button() == Qt::RightButton)
	{

	}
}

void QPropertyTree::keyPressEvent(QKeyEvent* ev)
{
	if (ev->key() == Qt::Key_F && ev->modifiers() == Qt::CTRL) {
      setFilterMode(true);
  }

  if (filterMode_) {
      if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
          setFilterMode(false);
      }
  }

	bool result = false;
	if (!widget_) {
		PropertyRow* row = model()->focusedRow();
		if (row)
			onRowKeyDown(row, ev);
	}
	update();
	if(!result)
		QWidget::keyPressEvent(ev);
}


void QPropertyTree::mouseDoubleClickEvent(QMouseEvent* ev)
{
	QWidget::mouseDoubleClickEvent(ev);

	QPoint point = ev->pos();
	PropertyRow* row = rowByPoint(point);
	if(row){
		if(row->widgetRect().contains(pointToRootSpace(point))){
			if(!row->onActivate(this, true) &&
				!row->onActivateRelease(this))
				toggleRow(row);	
		}
		else if(!toggleRow(row)) {
			row->onActivate(this, false);
			row->onActivateRelease(this);
		}
	}
}

void QPropertyTree::onMouseStill()
{
	if (capturedRow_) {
		PropertyDragEvent e;
		e.tree = this;
		e.pos = mapFromGlobal(QCursor::pos());
		e.start = pressPoint_;
	
		capturedRow_->onMouseStill(e);
	}
}

void QPropertyTree::mouseMoveEvent(QMouseEvent* ev)
{
	if(dragController_->captured() && !ev->buttons().testFlag(Qt::LeftButton))
		dragController_->interrupt();
	if(dragController_->captured()){
		QPoint pos = QCursor::pos();
		if (dragController_->dragOn(pos)) {
			// SetCapture
		}
		update();
	}
	else{
		QPoint point = ev->pos();
		PropertyRow* row = rowByPoint(pointToRootSpace(point));
		if(row){
			switch(hitTest(row, point, row->rect())){
			case TREE_HIT_ROW:
			//if(row != hoveredRow_){
				//hoveredRow_ = row;
				//RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			//}
			break;
			case TREE_HIT_PLUS:
			case TREE_HIT_TEXT:
			//if(hoveredRow_){
			//	hoveredRow_ = 0;
				//RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			//}
			break;
			case TREE_HIT_NONE:
			default:
			break;
			}
		}
		if(capturedRow_){
			onRowMouseMove(capturedRow_, QRect(), point);
			mouseStillTimer_->start(sliderUpdateDelay_);
		}
	}
}

void QPropertyTree::wheelEvent(QWheelEvent* ev) 
{
	QWidget::wheelEvent(ev);
	
	if (scrollBar_->isVisible() && scrollBar_->isEnabled())
 		scrollBar_->setValue(scrollBar_->value() + -ev->delta());
}

void QPropertyTree::updateArea()
{
}

bool QPropertyTree::toggleRow(PropertyRow* row)
{
	if(!row->canBeToggled(this))
		return false;
	expandRow(row, !row->expanded());

	updateHeights();
	return true;
}

bool QPropertyTree::_isDragged(const PropertyRow* row) const
{
	if (!dragController_->dragging())
		return false;
	if (dragController_->draggedRow() == row)
		return true;
	return false;
}

bool QPropertyTree::_isCapturedRow(const PropertyRow* row) const
{
	return capturedRow_ == row;
}



QPropertyTree::QPropertyTree(const QPropertyTree&)
{
}


QPropertyTree& QPropertyTree::operator=(const QPropertyTree&)
{
	return *this;
}

// vim:ts=4 sw=4:
