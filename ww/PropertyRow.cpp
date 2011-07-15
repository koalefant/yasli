#include "StdAfx.h"

#include "ww/PropertyTree.h"
#include "ww/TreeImpl.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PopupMenu.h"
#include "ww/_PropertyRowBuiltin.h"
#include "ww/PropertyDrawContext.h"
#include "yasli/TypesFactory.h"
#include "ww/Win32/Window.h"
#include "ww/Unicode.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Serialization.h"
#include "ww/Color.h"

#include "ww/Win32/Drawing.h"
#include "gdiplus.h"

namespace ww{

WW_API void* PropertyRowArg::object_(0);
WW_API size_t PropertyRowArg::size_(0);
WW_API const char* PropertyRowArg::name_(0);
WW_API const char* PropertyRowArg::nameAlt_(0);
WW_API const char* PropertyRowArg::typeName_(0);

// ---------------------------------------------------------------------------

inline unsigned calcHash(const char* str, unsigned hash = 5381)
{
	while(*str)
		hash = hash*33 + (unsigned char)*str++;
	return hash;
}

template<class T>
inline unsigned calcHash(const T& t, unsigned hash = 5381)
{
	for (int i = 0; i < sizeof(T); i++)
		hash = hash * 33 + ((unsigned char*)&t)[i];
	return hash;
}

// ---------------------------------------------------------------------------

ConstStringList* PropertyRow::constStrings_ = 0;

PropertyRow::PropertyRow()
{
    init("", "", "");
}

PropertyRow::PropertyRow(const char* name, const char* nameAlt, const char* typeName)
{
	init(name, nameAlt, typeName);
}

PropertyRow::PropertyRow(const char* name, const char* nameAlt, const Serializer &ser)
: serializer_(ser)
{
	init(name, nameAlt, ser.type().name());
}

void PropertyRow::init(const char* name, const char* nameAlt, const char* typeName)
{
	ASSERT(name != 0);
	ASSERT(typeName);

	parent_ = 0;

	expanded_ = false;
	selected_ = false;
	visible_ = true;
	labelUndecorated_ = 0;
	labelChanged_ = true;
    belongsToFilteredRow_ = false;
    matchFilter_ = true;
	
	pos_ = size_ = Vect2::ZERO;
	plusSize_ = 0;
	textPos_ = 0;
	textSizeInitial_ = 0;
	textHash_ = 0;
	textSize_ = 0;
	widgetPos_ = 0;
	digestPos_ = 0;
    widgetSize_ = 0;
	widgetSizeMin_ = 0;
	freePulledChildren_ = 0;
	textSizeDelta_ = 0;
	widgetSizeDelta_ = 0;
	
	name_ = name[0] || !nameAlt ? name : nameAlt;
	typeName_ = typeName;
	
	updated_ = false;
	pulledUp_ = false;
	pulledBefore_ = false;
	hasPulled_ = false;
	readOnly_ = false;
	readOnlyOver_ = false;
	fullRow_ = false;
	multiValue_ = false;
	
	setLabel(nameAlt ? TRANSLATE(nameAlt) : 0);
}


PropertyRow* PropertyRow::childByIndex(int index)
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

const PropertyRow* PropertyRow::childByIndex(int index) const
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

void PropertyRow::_setExpanded(bool expanded)
{
    expanded_ = expanded;
	Rows::iterator i;
	FOR_EACH(children_, i)
		if((*i)->pulledUp())
			(*i)->_setExpanded(expanded);
}

void PropertyRow::setExpandedRecursive(PropertyTree* tree, bool expanded)
{
	if(canBeToggled(tree))
		_setExpanded(expanded);
	
	struct Op {
		bool expanded_;
		Op(bool expanded) : expanded_(expanded) {}
		ScanResult operator()(PropertyRow* row, PropertyTree* tree)
		{
			if(row->canBeToggled(tree))
				row->_setExpanded(expanded_);
			return SCAN_CHILDREN_SIBLINGS;
		}
	};
	scanChildren(Op(expanded), tree);
}

int PropertyRow::childIndex(PropertyRow* row)
{
	ASSERT(row);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	ESCAPE(it != children_.end(), return -1);
	return std::distance(children_.begin(), it);
}

bool PropertyRow::isChildOf(const PropertyRow* row) const
{
	const PropertyRow* p = parent();
	while(p){
		if(p == row)
			return true;
		p = p->parent();
	}
	return false;
}

PropertyRow* PropertyRow::add(PropertyRow* row, PropertyRow* after)
{
	if(after == 0 || after == this)
		children_.push_back(row);
	else{
		iterator it = std::find(children_.begin(), children_.end(), after);
		if(it != children_.end()){
			++it;
			children_.insert(it, row);
		}
		else{
			children_.push_back(row);
		}
	}
	row->setParent(this);
	return row;
}

void PropertyRow::assignRowState(const PropertyRow& row, bool recurse)
{
	expanded_ = row.expanded_;
	selected_ = row.selected_;
    if(recurse){

        Rows::iterator it;
        for(it = children_.begin(); it != children_.end(); ++it){
            PropertyRow* child = it->get();
            ESCAPE(child, continue);
            if(child->name()[0] != '\0'){
                const PropertyRow* rhsChild = row.find(child->name(), child->label(), child->typeName());
                if(rhsChild)
                    child->assignRowState(*rhsChild, true);
            }
            else{
                int index = std::distance(children_.begin(), it);
                if(size_t(index) < row.count())
                    child->assignRowState(*row.childByIndex(index), true);
            }
        }
    }
}

void PropertyRow::assignRowProperties(PropertyRow* row)
{
    ESCAPE(row, return);
	parent_ = row->parent_;
	
	readOnly_ = row->readOnly_;
	readOnlyOver_ = row->readOnlyOver_;
	userFixedWidget_ = row->userFixedWidget_;
	pulledUp_ = row->pulledUp_;
	pulledBefore_ = row->pulledBefore_;
	size_ = row->size_;
	pos_ = row->pos_;
	plusSize_ = row->plusSize_;
	textPos_ = row->textPos_;
	textSizeInitial_ = row->textSizeInitial_;
	textHash_ = row->textHash_;
	textSize_ = row->textSize_;
	widgetPos_ = row->widgetPos_;
	widgetSize_ = row->widgetSize_;
	widgetSizeMin_ = row->widgetSizeMin_;
	freePulledChildren_ = row->freePulledChildren_;
	textSizeDelta_ = row->textSizeDelta_;
	widgetSizeDelta_ = row->widgetSizeDelta_;

    assignRowState(*row, false);
}

void PropertyRow::replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, bool preserveChildren)
{
	Rows::iterator it = std::find(children_.begin(), children_.end(), oldRow);
	ASSERT(it != children_.end());
	if(it != children_.end()){
		newRow->assignRowProperties(*it);
		if(preserveChildren)
			(*it)->swapChildren(newRow);
		*it = newRow;
	}
}

void PropertyRow::erase(PropertyRow* row)
{
	row->setParent(0);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	ASSERT(it != children_.end());
	if(it != children_.end())
		children_.erase(it);
}

void PropertyRow::swapChildren(PropertyRow* row)
{
	children_.swap(row->children_);
	iterator it;
	for( it = children_.begin(); it != children_.end(); ++it)
		(**it).setParent(this);
	for( it = row->children_.begin(); it != row->children_.end(); ++it)
		(**it).setParent(row);
}

PropertyRow* PropertyRow::addBefore(PropertyRow* row, PropertyRow* before)
{
	if(before == 0)
		children_.push_back(row);
	else{
		iterator it = std::find(children_.begin(), children_.end(), before);
		if(it != children_.end())
			children_.insert(it, row);
		else
			children_.push_back(row);
	}
	row->setParent(this);
	return row;
}

void PropertyRow::digestAppend(const wchar_t* text)
{
	if(!digest_.empty())
		digest_ += L", ";
	digest_ += text;
}

void PropertyRow::digestReset()
{
	digest_.clear();
}

std::wstring PropertyRow::valueAsWString() const
{
    return toWideChar(valueAsString().c_str());
}

std::string PropertyRow::valueAsString() const
{
	if(!digest_.empty())
		return std::string("( ") + fromWideChar(digest_.c_str()) + " )";
	else
		return "";
}

PropertyRow* PropertyRow::cloneChildren(PropertyRow* result, const PropertyRow* source) const
{
	PropertyRow::const_iterator it;
	for(it = source->begin(); it != source->end(); ++it){
		const PropertyRow* sourceChild = *it;
		result->add(sourceChild->clone());
	}

	return result;
}

void PropertyRow::drawStaticText(Gdiplus::Graphics* gr, const Gdiplus::Rect& widgetRect)
{
}

void PropertyRow::serialize(Archive& ar)
{
	serializeValue(ar);

	ar.serialize(ConstStringWrapper(constStrings_, name_), "name", "&name");
	ar.serialize(ConstStringWrapper(constStrings_, label_), "nameAlt", "nameAlt");
	ar.serialize(ConstStringWrapper(constStrings_, typeName_), "type", "type");
	ar.serialize(reinterpret_cast<std::vector<SharedPtr<PropertyRow> >&>(children_), "children", "!^children");	
	if(ar.isInput()){
		setLabelChanged();
		PropertyRow::iterator it;
		for(it = begin(); it != end(); ){
			PropertyRow* row = *it;
			if(row){
				row->setParent(this);
				++it;
			}
			else{
				it = erase(it);
			}
		}
	}
}

bool PropertyRow::onActivate( PropertyTree* tree, bool force)
{
    return tree->spawnWidget(this, force);
}

void PropertyRow::setLabelChanged() 
{ 
	labelChanged_ = true;  
	for(PropertyRow* row = parent(); row; row = row->parent())
		row->labelChanged_ = true;
}

void PropertyRow::updateLabel()
{
	if(!labelChanged_)
		return;
	labelChanged_ = false;

	digestReset();

	PropertyRow::iterator it;
    FOR_EACH(children_, it)
		(*it)->updateLabel();

	parseControlCodes(label_);
	visible_ = *labelUndecorated_ || fullRow_ || pulledUp_ || isRoot();
	if(!*label_)
		labelUndecorated_ = name();

	if(pulledContainer())
		pulledContainer()->_setExpanded(expanded());
}

void PropertyRow::parseControlCodes(const char* ptr)
{
	fullRow_ = false;
	pulledUp_ = pulledBefore_ = false;
	hasPulled_ = false;
	readOnly_ = readOnlyOver_ = false;
	userFixedWidget_ = false;

	bool appendValueToDigest = false;
	while(true){
		if(*ptr == '^'){
			if(parent() && !parent()->isRoot()){
				if(pulledUp_)
					pulledBefore_ = true;
				pulledUp_ = true;
				parent()->hasPulled_ = true;

				if(pulledUp() && isContainer())
					parent()->setPulledContainer(this);
			}
		}
		else if(*ptr == '+')
			_setExpanded(true);
		else if(*ptr == '<')
			fullRow_ = true;
		else if(*ptr == '>'){
			userFixedWidget_ = true;
			const char* p = ++ptr;
			while(*p >= '0' && *p <= '9')
				++p;
			if(*p == '>'){
				widgetSizeMin_ = atoi(ptr);
				ptr = ++p;
			}
			continue;
		}
		else if(*ptr == '&')
			appendValueToDigest = true;
		else if(*ptr == '~'){
			struct Op{ ScanResult operator()(PropertyRow* row) { row->serializer_ = Serializer(); return SCAN_CHILDREN_SIBLINGS; } };
			scanChildren(Op());
		}
		else if(*ptr == '!'){
			if(readOnly_)
				readOnlyOver_ = true;
			readOnly_ = true;
		}
		else if(*ptr == '['){
			++ptr;
			PropertyRow::iterator it;
			FOR_EACH(children_, it)
				(*it)->parseControlCodes(ptr);

			int counter = 1;
			while(*ptr){
				if(*ptr == ']' && !--counter)
					break;
				else if(*ptr == '[')
					++counter;
				++ptr;
			}
		}
		else
			break;
		++ptr;
	}

	labelUndecorated_ = ptr;
	if (appendValueToDigest)
		parent()->digestAppend(valueAsWString().c_str());
}

const char* PropertyRow::typeNameForFilter() const
{
	const char* typeName = this->typeName();
#ifdef _MSC_VER
	if (strncmp(typeName, "struct ", 7) == 0)
		typeName += 7;
	else if(strncmp(typeName, "class ", 6) == 0)
		typeName += 6;
	else if(strncmp(typeName, "enum ", 5) == 0)
		typeName += 5;

#endif
	if(strncmp(typeName, "yasli::", 7) == 0)
		typeName += 7;
	else if(strncmp(typeName, "ww::", 4) == 0)
		typeName += 4;
	else if(strncmp(typeName, "std::", 5) == 0)
		typeName += 5;
	
	return typeName;
}


void PropertyRow::updateSize(const PropertyTree* tree)
{
	updateLabel();

	plusSize_ = 0;
    if(isRoot())
        expanded_ = true;
	else{
		if(nonPulledParent()->isRoot() || tree->compact() && nonPulledParent()->parent()->isRoot())
			_setExpanded(true);
		else if(!pulledUp())
			plusSize_ = tree->tabSize();

		if(parent()->pulledUp())
			pulledBefore_ = false;

		if(!visible(tree) && !(isContainer() && pulledUp())){
			size_.set(0, 0);
			return;
		}
	}

	std::string text = rowText();
	if(text.empty())
		textSizeInitial_ = 0;
	else{
		unsigned hash = calcHash(text.c_str());
		Gdiplus::Font* font = rowFont(tree);
		hash = calcHash(font, hash);
		if(hash != textHash_){
			textHash_ = hash;
			HDC dc = GetDC(*Win32::_globalDummyWindow);
			Gdiplus::Graphics gr(dc);
			std::wstring wstr(toWideChar(text.c_str()));
			Gdiplus::StringFormat format;
			Gdiplus::RectF bound;
			gr.MeasureString(wstr.c_str(), (int)wstr.size(), font, Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f), &format, &bound, 0);
			ReleaseDC(*Win32::_globalDummyWindow, dc);
			textSizeInitial_ = bound.Width + 3;
		}
    }

	textSize_ = textSizeInitial_;
	widgetSize_ = widgetSizeMin();
	if(pulledUp() && isContainer())
	{
		//textSize_ = widgetSize_ = plusSize_ = 0;
		textSize_ = plusSize_ = 0;
		widgetSize_ = widgetSizeMin_;
	}

	size_.set(plusSize_ + textSize_ + widgetSize_, ROW_DEFAULT_HEIGHT + floorHeight());
	textSizeDelta_ = textSize_ > TEXT_SIZE_MIN ? textSize_ - TEXT_SIZE_MIN : 0;
	widgetSizeDelta_ = widgetSize_ > WIDGET_SIZE_MIN ? widgetSize_ - WIDGET_SIZE_MIN : 0;
	bool hasResizeableWidget = (hasWidgetAt(WIDGET_POS_VALUE) && !isWidgetFixed(WIDGET_POS_VALUE)) || 
		                       (hasWidgetAt(WIDGET_POS_VALUE) && !isWidgetFixed(WIDGET_POS_ICON)) || 
							   (hasWidgetAt(WIDGET_POS_AFTER_NAME) && !isWidgetFixed(WIDGET_POS_AFTER_NAME));
	freePulledChildren_ = widgetSize_ && hasResizeableWidget ? 1 : 0;
    Rows::iterator it;
    FOR_EACH(children_, it){
		PropertyRow* row = *it;
		if(!row->visible(tree))
			continue;
        row->updateSize(tree);
		if(row->pulledUp()){
			size_.x += row->size_.x;
			size_.y = max(size_.y, row->size_.y);
			textSizeDelta_ += row->textSizeDelta_;
			widgetSizeDelta_ += row->widgetSizeDelta_;
			freePulledChildren_ += row->freePulledChildren_;
		}
    }
}

void PropertyRow::adjustRect(const PropertyTree* tree, const Rect& rect, Vect2 pos, int& totalHeight, int& _extraSize)
{
	pos_ = pos;
	pos.x += plusSize_;

	int extraSizeStorage = 0;
	int& extraSize = !pulledUp() ? extraSizeStorage : _extraSize;
	if(!pulledUp()){
		extraSize = rect.width() - size_.x - pos.x;
		if(extraSize < 0){
			if(-extraSize > textSizeDelta_){
				scaleSize(0, false);
				extraSize += textSizeDelta_;
			}
			else{
				scaleSize(float(textSizeDelta_ + extraSize)/textSizeDelta_, false);
				extraSize = 0;
			}

			if(-extraSize > widgetSizeDelta_){
				scaleSize(0, true);
				extraSize += widgetSizeDelta_;
			}
			else{
				scaleSize(float(widgetSizeDelta_ + extraSize)/widgetSizeDelta_, true);
				extraSize = 0;
			}

			cutSize(extraSize, false);
			cutSize(extraSize, true);
		}
	}

	if(hasWidgetAt(WIDGET_POS_ICON)){
		widgetPos_ = widgetSize_ ? pos.x : -1000;
		pos.x += widgetSize_;
		textPos_ = pos.x;
		pos.x += textSize_;
	}

	Rows::iterator it;
	FOR_EACH(children_, it)
		if((*it)->pulledBefore()){
			(*it)->adjustRect(tree, rect, pos, totalHeight, _extraSize);
			pos.x += (*it)->size_.x;
		}

	if(!hasWidgetAt(WIDGET_POS_ICON)){
		textPos_ = pos.x;
		pos.x += textSize_;
	}

	if(hasWidgetAt(WIDGET_POS_AFTER_NAME)){
		widgetPos_ = pos.x;
		pos.x += widgetSize_;
	}

	if(!pulledUp() && extraSize > 0/* && hasWidgetAt(WIDGET_POS_VALUE)*/){
		int widgetsSize = size_.x - pos.x + pos_.x;
		if(!fullRow(tree))
			pos.x = max(rect.left() + round(rect.width()*(1.f - tree->valueColumnWidth())), pos.x);
		extraSize = rect.right() - pos.x - widgetsSize;
		if(extraSize > 0)
			extraSize = freePulledChildren_ > 0 ? extraSize/freePulledChildren_ : extraSize;
		else{
			pos.x = max(pos.x + extraSize, textPos_ + (parent()->isContainer() ? textSize_ : 0));
			extraSize = 0;
		}
	}

	if(hasWidgetAt(WIDGET_POS_VALUE)){
		if(widgetSize_ && !isWidgetFixed(WIDGET_POS_VALUE) && extraSize > 0){
			widgetSize_ += extraSize;
			size_.x += extraSize;
		}

		widgetPos_ = pos.x;
		pos.x += widgetSize_;
		int delta = rect.right() - pos.x;
		if(delta > 0 && delta < 4)
			widgetSize_ += delta;
	}

	size_.x = textSize_ + widgetSize_;

	PropertyRow* nonPulled = nonPulledParent();
	bool hasChildrenPulledAfter = false;
	FOR_EACH(children_, it){
        PropertyRow* row = *it;
        if(row->pulledUp()){
			if(row->pulledBefore())
				continue;
			row->adjustRect(tree, rect, pos, totalHeight, extraSize);
			//if(!row->isContainer()){
				pos.x += row->size_.x;
				size_.x += row->size_.x;
			//}
			hasChildrenPulledAfter = true;
        }
		else if(row->visible(tree) && nonPulled->expanded()){
			Vect2 rowPos(nonPulled->plusRect().right(), totalHeight);
			totalHeight += row->size_.y;
			row->adjustRect(tree, rect, rowPos, totalHeight, extraSize);
        }
    }

	digestPos_ = pos.x;

	if(!pulledUp())
	{
		// widget at end
		//if (!hasChildrenPulledAfter && hasWidgetAt(WIDGET_POS_ICON))
		//	widgetPos_ = rect.right() - widgetSize_;
		size_.x = rect.right() - pos_.x;
	}
}

void PropertyRow::scaleSize(float t, bool scaleWidget)
{
	if(!scaleWidget){
		if(textSize_ > TEXT_SIZE_MIN)
			textSize_ = TEXT_SIZE_MIN + round((textSize_ - TEXT_SIZE_MIN)*t);
	}
	else{
		if(widgetSize_ > WIDGET_SIZE_MIN)
			widgetSize_ = WIDGET_SIZE_MIN + round((widgetSize_ - WIDGET_SIZE_MIN)*t);
	}

	Rows::iterator i;
	FOR_EACH(children_, i)
		if((*i)->pulledUp())
			(*i)->scaleSize(t, scaleWidget);
}

void PropertyRow::cutSize(int& extraSize, bool cutWidget)
{
	if(extraSize >= 0)
		return;

	if(!cutWidget){
		if(-extraSize > textSize_){
			extraSize += textSize_;
			textSize_ = 0;
		}
		else{
			textSize_ += extraSize;
			extraSize = 0;
		}
	}

	for(Rows::reverse_iterator i = children_.rbegin(); i != children_.rend(); i++)
		if((*i)->pulledUp())
			(*i)->cutSize(extraSize, cutWidget);

	if(cutWidget){
		if(-extraSize > widgetSize_){
			extraSize += widgetSize_;
			widgetSize_ = 0;
		}
		else{
			widgetSize_ += extraSize;
			extraSize = 0;
			if(hasWidgetAt(WIDGET_POS_ICON))
				widgetSize_ = 0;
		}
	}
}

PropertyRow* PropertyRow::findSelected()
{
    if(selected())
        return this;
    iterator it;
    FOR_EACH(children_, it){
        PropertyRow* result = (*it)->findSelected();
        if(result)
            return result;
    }
    return 0;
}

PropertyRow* PropertyRow::find(const char* name, const char* nameAlt, const char* typeName, bool skipUpdated)
{
	iterator it;
	FOR_EACH(children_, it){
		PropertyRow* row = *it;
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
		   ((nameAlt == 0) || (row->label() != 0 && strcmp(row->label(), nameAlt) == 0)) &&
		   ((typeName == 0) || (row->typeName() != 0 && strcmp(row->typeName(), typeName) == 0)) &&
		   (!skipUpdated || !row->updated()))
			return row;
	}
	return 0;
}

const PropertyRow* PropertyRow::find(const char* name, const char* nameAlt, const char* typeName, bool skipUpdated) const
{
	return const_cast<PropertyRow* const>(this)->find(name, nameAlt, typeName, skipUpdated);
}

bool PropertyRow::onKeyDown(PropertyTree* tree, KeyPress key)
{
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = safe_cast<PropertyRowContainer*>(parent());
		if(key == KeyPress(KEY_DELETE)){
			container->onMenuChildRemove(this, tree->model());
			return true;
		}
		if(key == KeyPress(KEY_INSERT, KEY_MOD_SHIFT)){
			container->onMenuChildInsertBefore(this, tree);
			return true;
		}
	}
	return false;
}

bool PropertyRow::onContextMenu(PopupMenuItem &root, PropertyTree* tree)
{
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = safe_cast<PropertyRowContainer*>(parent());
        if(!container->readOnly()){
		    if(!root.empty())
			    root.addSeparator();
		    root.add(TRANSLATE("Insert Before"), this, tree)
			    .connect(container, &PropertyRowContainer::onMenuChildInsertBefore)
			    .setHotkey(KeyPress(KEY_INSERT, KEY_MOD_SHIFT))
			    .enable(!container->readOnly());
		    root.add(TRANSLATE("Remove"), this, tree->model())
			    .connect(container, &PropertyRowContainer::onMenuChildRemove)
			    .setHotkey(KeyPress(KEY_DELETE, KEY_MOD_SHIFT))
			    .enable(!container->readOnly());
        }
	}

	if(hasVisibleChildren(tree)){
	    if(!root.empty())
		    root.addSeparator();

		root.add(TRANSLATE("Expand"), this).connect(tree, &PropertyTree::expandAll);
		root.add(TRANSLATE("Collapse"), this).connect(tree, &PropertyTree::collapseAll);
    }

	return !root.empty();
}

int PropertyRow::level() const
{
    int result = 0;
    const PropertyRow* row = this;
    while(row){
        row = row->parent();
        ++result;
    }
    return result;
}

PropertyRow* PropertyRow::nonPulledParent()
{
	PropertyRow* row = this;
	while(row->pulledUp())
		row = row->parent();
	return row;
}

bool PropertyRow::pulledSelected() const
{
	if(selected())
		return true;
	const PropertyRow* row = this;
	while(row->pulledUp()){
		row = row->parent();
		if(row->selected())
			return true;
	}
	return false;
}


Gdiplus::Font* PropertyRow::rowFont(const PropertyTree* tree) const
{
	Gdiplus::Font* normalFont = propertyTreeDefaultFont();
	Gdiplus::Font* boldFont = propertyTreeDefaultBoldFont();
	return hasVisibleChildren(tree) || isContainer() ? boldFont : normalFont;
}

void PropertyRow::drawRow(HDC dc, const PropertyTree* tree) 
{
    if(!visible(tree))
        return;

    using namespace Gdiplus;
	using Gdiplus::Rect;
	using Gdiplus::Color;

    Graphics gr(dc);
    gr.SetSmoothingMode(SmoothingModeAntiAlias);
    gr.SetTextRenderingHint(TextRenderingHintSystemDefault);

	PropertyDrawContext context;
	context.graphics = &gr;
	context.tree = tree;
	context.widgetRect = widgetRect();
   	context.lineRect = floorRect();

	::ww::Color textColor;
	textColor.setGDI(GetSysColor(COLOR_BTNTEXT));

	ww::Rect rowRect = rect();

    // drawing a horizontal line
    std::wstring text = toWideChar(rowText().c_str());

    if(textSize_ && widgetSize_ && !isStatic() && hasWidgetAt(WIDGET_POS_VALUE) && !pulledUp() && !fullRow(tree) && !hasPulled()){
		Color color1;
        color1.SetFromCOLORREF(GetSysColor(COLOR_BTNFACE));
        Color color2;
        color2.SetFromCOLORREF(GetSysColor(COLOR_3DDKSHADOW));
		Rect rect = gdiplusRect(rowRect);
        rect.X = textPos_ - 1;
        rect.Y = rowRect.bottom() - 3;
        rect.Height = 1;
        rect.Width = rowRect.width() - textPos_ - 3;
        Gdiplus::LinearGradientBrush gradientBrush(rect, color1, color2, Gdiplus::LinearGradientModeHorizontal);
        rect.X += 1;
        rect.Width -= 2;
        gradientBrush.SetWrapMode(Gdiplus::WrapModeClamp);
        gr.FillRectangle(&gradientBrush, rect);
    }

    Rect selectionRect = gdiplusRect(rowRect);
    if(!pulledUp()){
        selectionRect.X = rowRect.left() + plusSize_ - (tree->compact() ? 1 : 2);
        selectionRect.Width = rowRect.right() - selectionRect.X;
    }
    else{
        selectionRect.X = rowRect.left() - 1;
        selectionRect.Y += 1;
        selectionRect.Width = rowRect.width() + 1;
        selectionRect.Height -= 2;
    }

    if(selected()){
		// drawing selection rectangle
        if(tree->isFocused()){
            Color brushColor;
            brushColor.SetFromCOLORREF(GetSysColor(COLOR_HIGHLIGHT));
            SolidBrush brush(brushColor);
            Color borderColor(brushColor.GetA() / 4, brushColor.GetR(), brushColor.GetG(), brushColor.GetB());
            fillRoundRectangle( &gr, &brush, selectionRect, borderColor, 6 );
        }
        else{
            Color brushColor;
            brushColor.SetFromCOLORREF(GetSysColor(COLOR_3DDKSHADOW));
            SolidBrush brush(brushColor);
            Color borderColor(brushColor.GetA() / 4, brushColor.GetR(), brushColor.GetG(), brushColor.GetB());
            fillRoundRectangle( &gr, &brush, selectionRect, borderColor, 6 );
        }
    }
    if(pulledSelected())
		textColor.setGDI(GetSysColor(COLOR_HIGHLIGHTTEXT));

    if(!tree->compact() || !parent()->isRoot()){
        if(hasVisibleChildren(tree)){
			drawPlus(&gr, plusRect(), expanded(), selected(), expanded());
        }
    }

    // custom drawing
	if(!isStatic())
		redraw(context);

    if(textSize_ > 0){
        Font* font = rowFont(tree);
		tree->_drawRowLabel(&gr, text.c_str(), font, textRect(), textColor);
    }

    // drawing digest
    if(!digest_.empty()){
		int digestPos = digestPos_;
        int width = 0;
        if(widgetSize_ == 0)
            width = rowRect.right() - digestPos;
        else
            width = max(0, widgetPos_ - digestPos);

        if(width > 0){
			StringFormat format;
			format.SetAlignment(StringAlignmentNear);
			format.SetLineAlignment(StringAlignmentCenter);
			format.SetTrimming(StringTrimmingEllipsisCharacter);
			format.SetFormatFlags(StringFormatFlagsNoWrap);

            RectF digestRect(digestPos, pos_.y, width, ROW_DEFAULT_HEIGHT);
            gr.DrawString(digest_.c_str(), (int)digest_.size(), propertyTreeDefaultFont(), digestRect, &format, &SolidBrush(gdiplusSysColor(COLOR_3DSHADOW)));
        }
    }
}

void PropertyRow::drawPlus(Gdiplus::Graphics* gr, const Rect& rect, bool expanded, bool selected, bool grayed) const
{	
	using namespace Gdiplus;
	Vect2 size(9, 9);
	Vect2 center(rect.center());
	Win32::Rect r(Rect(center - Vect2(4, 4), center - Vect2(4, 4) + size));

	fillRoundRectangle(gr, &SolidBrush(gdiplusSysColor(/*grayed ? COLOR_BTNFACE : */COLOR_WINDOW)), gdiplusRect(r), gdiplusSysColor(COLOR_3DDKSHADOW), 3);

	gr->DrawLine(&Pen(gdiplusSysColor(COLOR_3DDKSHADOW)), center.x - 2, center.y, center.x + 2, center.y);
	if(!expanded)
		gr->DrawLine(&Pen(gdiplusSysColor(COLOR_3DDKSHADOW)), center.x, center.y - 2, center.x, center.y + 2);
}

bool PropertyRow::visible(const PropertyTree* tree) const
{
	return ((visible_ || !tree->hideUntranslated()) && (matchFilter_ || belongsToFilteredRow_));
}

bool PropertyRow::fullRow(const PropertyTree* tree) const
{
	return fullRow_ || tree->fullRowMode();
}

bool PropertyRow::canBeToggled(const PropertyTree* tree) const
{
	if(!visible(tree))
		return false;
	if((tree->compact() && parent()->isRoot()) || isContainer() && pulledUp() || !hasVisibleChildren(tree))
		return false;
	return !empty();
}

bool PropertyRow::canBeDragged() const
{
	if(parent()){
		if(parent()->isContainer())
			return true;
	}
	return false;
}

bool PropertyRow::canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const
{
	ASSERT(parentRow);

	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();

	if(parentRow->isContainer()){
		const PropertyRowContainer* container = safe_cast<const PropertyRowContainer*>(parentRow);
				
		if((container->isFixedSize() || container->readOnly()) && parent() != parentRow)
			return false;

		if(beforeChild && beforeChild->parent() != parentRow)
			return false;

		const PropertyRow* defaultRow = container->defaultRow(tree->model());
		if(defaultRow && strcmp(defaultRow->typeName(), typeName()) == 0)
			return true;
	}
	return false;	
}

void PropertyRow::dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before)
{
	SharedPtr<PropertyRow> ref(this);

	PropertyTreeModel* model = tree->model();
	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();
	if(parentRow->isContainer()){
        tree->model()->push(tree->model()->root()); // FIXME: выбрать оптимальный узел
		PropertyRowContainer* container = safe_cast<PropertyRowContainer*>(parentRow);
		PropertyRow* oldParent = parent();
		oldParent->erase(this);
		if(before)
			parentRow->addBefore(this, cursorRow);
		else
			parentRow->add(this, cursorRow);
		model->selectRow(this, true);
        TreePath thisPath = tree->model()->pathFromRow(this);
        model->rowChanged(tree->model()->root()); // after this call we can get invalid this
        if(PropertyRow* newThis = tree->model()->rowFromPath(thisPath)) // we use path to obtain new row
            tree->ensureVisible(newThis);
	}
}

void PropertyRow::intersect(const PropertyRow* row)
{
	setMultiValue(valueAsString() != row->valueAsString());

	iterator it = begin();
	const_iterator it2 = row->begin();
	for(; it != end();){
		if(it2 == row->end() || strcmp((*it)->typeName(), (*it2)->typeName()) != 0)
			it = children_.erase(it);
		else{
			(*it)->intersect(*it2);
			++it;
			if(it2 != row->end())
				++it2;
		}
	}
}

std::string PropertyRow::rowText() const
{
	if(parent() && parent()->isContainer()){
		std::string text;
		int index = 0;
		PropertyRow::const_iterator it = parent()->begin();
		while(it != parent()->end() && &**it != this){
			++index;
			++it;
		}
		char buffer[15];
		sprintf_s(buffer, " %i.", index);
		text = buffer;
		return text;
	}
	else
		return labelUndecorated();
}

bool PropertyRow::hasVisibleChildren(const PropertyTree* tree, bool internalCall) const
{
	if(empty() || !internalCall && pulledUp())
		return false;

	PropertyRow::const_iterator it;
	FOR_EACH(children_, it){
		const PropertyRow* child = *it;
		if(child->pulledUp()){
            if(child->hasVisibleChildren(tree, true))
                return true;
        }
        else if(child->visible(tree))
                return true;
	}
	return false;
}

const PropertyRow* PropertyRow::hit(const PropertyTree* tree, Vect2 point) const
{
  return const_cast<PropertyRow*>(this)->hit(tree, point);
}

PropertyRow* PropertyRow::hit(const PropertyTree* tree, Vect2 point)
{
    bool expanded = this->expanded();
    if(isContainer() && pulledUp())
        expanded = parent() ? parent()->expanded() : true;
    bool onlyPulled = !expanded;
    PropertyRow::const_iterator it;
    FOR_EACH(children_, it){
        PropertyRow* child = *it;
		if (!child->visible(tree))
			continue;
        if(!onlyPulled || child->pulledUp())
            if(PropertyRow* result = child->hit(tree, point))
                return result;
    }
	if(Rect(pulledUp() ? pos_.x : 0, pos_.y, pos_.x + size_.x, pos_.y + size_.y).pointInside(point))
        return this;
    return 0;
}

PropertyRow* PropertyRow::findByAddress(void* addr)
{
    if(serializer_.pointer() == addr)
        return this;
    else{
        Rows::iterator it;
        for(it = children_.begin(); it != children_.end(); ++it){
            PropertyRow* result = it->get()->findByAddress(addr);
            if(result)
                return result;
        }
    }
    return 0;
}

int PropertyRow::verticalIndex(PropertyTree* tree, PropertyRow* row)
{
	struct Op{
		int index_;
		const PropertyRow* row_;

		Op(const PropertyRow* row) : row_(row), index_(0) {}

		ScanResult operator()(PropertyRow* row, PropertyTree* tree)
		{
			if(row == row_)
				return SCAN_FINISHED;
			if(row->visible(tree) && row->isSelectable() && !row->pulledUp())
				++index_;
			return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
		}
	};

	Op op(row);
	scanChildren(op, tree);
	return op.index_;
}

PropertyRow* PropertyRow::rowByVerticalIndex(PropertyTree* tree, int index)
{
	struct Op{
		int index_;
		PropertyRow* row_;

		Op(int index) : row_(0), index_(index) {}

		ScanResult operator()(PropertyRow* row, PropertyTree* tree)
		{
			if(row->visible(tree) && !row->pulledUp() && row->isSelectable()){
				row_ = row;
				if(index_-- <= 0)
					return SCAN_FINISHED;
			}
			return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
		}
	};

	Op op(index);
	scanChildren(op, tree);
	return op.row_;
}

int PropertyRow::horizontalIndex(PropertyTree* tree, PropertyRow* row)
{
	struct Op{
		int index_;
		PropertyRow* row_;
		bool pulledBefore_;

		Op(PropertyRow* row) : row_(row), index_(0), pulledBefore_(row->pulledBefore()) {}

		ScanResult operator()(PropertyRow* row, PropertyTree* tree)
		{
			if(!row->pulledUp())
				return SCAN_SIBLINGS;
			if(row->visible(tree) && row->pulledUp() && row->pulledBefore() == pulledBefore_){
				index_ += pulledBefore_ ? -1 : 1;
				if(row == row_)
					return SCAN_FINISHED;
			}
			return SCAN_CHILDREN_SIBLINGS;
		}
	};

	if(row == this)
		return 0;
	Op op(row);
	if(row->pulledBefore())
		scanChildrenReverse(op, tree);
	else
		scanChildren(op, tree);
	return op.index_;
}

PropertyRow* PropertyRow::rowByHorizontalIndex(PropertyTree* tree, int index)
{
	struct Op{
		int index_;
		PropertyRow* row_;
		bool pulledBefore_;

		Op(int index) : row_(0), index_(index), pulledBefore_(index < 0) {}

		ScanResult operator()(PropertyRow* row, PropertyTree* tree)
		{
			if(!row->pulledUp())
				return SCAN_SIBLINGS;
			if(row->visible(tree) && row->pulledUp() && row->pulledBefore() == pulledBefore_){
				row_ = row;
				if(pulledBefore_ ? ++index_ >= 0 : --index_ <= 0)
					return SCAN_FINISHED;
			}
			return SCAN_CHILDREN_SIBLINGS;
		}
	};

	if(!index)
		return this;
	Op op(index);
	if(index < 0)
		scanChildrenReverse(op, tree);
	else
		scanChildren(op, tree);
	return op.row_ ? op.row_ : this;
}

void PropertyRow::redraw(const PropertyDrawContext& context)
{

}


YASLI_CLASS(PropertyRow, PropertyRow, "Структура");

// ---------------------------------------------------------------------------

PropertyRowWidget::~PropertyRowWidget()
{
	if(actualWidget())
		actualWidget()->_setParent(0);
}

FORCE_SEGMENT(PropertyRowDecorators)
FORCE_SEGMENT(PropertyRowBitVector)
FORCE_SEGMENT(PropertyRowFileSelector)
FORCE_SEGMENT(PropertyRowColor)
FORCE_SEGMENT(PropertyRowHotkey)
FORCE_SEGMENT(PropertyRowSlider)

}

// [] boolLabel0 [] boolLabel1 HostLabel [hostValue] intLabel0 [intValue0] [] boolLabel3 intLabel1 [intValue1] Digest

