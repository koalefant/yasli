/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/ClassFactory.h"
#include <string>
#include "ww/sigslot.h
#include "ww/MenuBar.h"
#include "ww/VBox.h"
#include "ww/SpaceHeader.h"

template<class BaseType, class FactoryArg>
class SerializationFactory;

namespace ww{

class Workspace;
class Widget;
class CommandManager;
class ScrolledWindow;
class HBox;
class Space : public RefCounter, public sigslot::has_slots{
public:
	typedef SerializationFactory<Space, FactoryArg0<Space> > Factory;
	Space();
	virtual ~Space();

	virtual void add(Widget* widget);
	virtual Widget* widget();

	void addToHeader(Widget* widget);

	void setMenu(const char* menuPath);
	virtual Space* spaceByPoint(Vect2 point);
	virtual bool selfSplit(Vect2 screenPoint) { return false; }
	Space* clone();
	virtual void replaceSpace(Space* oldSpace, Space* newSpace);
	virtual bool isLeaf() const{ return true; }

	virtual void serialize(Archive& ar);
	
	void setPosition(float position){ position_ = position; }
	float position() const{ return position_; }

	Widget*	findWidget();
	Workspace* findWorkspace();

	Space* parent() { return parent_; }
	virtual void setParent(Space* space);

	CommandManager& commands() { return *commandManager_; };
protected:
	void rebuildWidgets();

	SharedPtr<VBox> vbox_;
	SharedPtr<SpaceHeader> header_;
	SharedPtr<MenuBar> menuBar_;
	SharedPtr<Widget> widget_;
	HBox* headerBox_;
	ScrolledWindow* scrolledWindow_;

	typedef std::vector<SharedPtr<Widget> > Widgets;
	Widgets headerWidgets_;

	CommandManager* commandManager_;
	std::string menuPath_;

	Space* parent_;

	Space(float position);
	float position_;
};

typedef ::Factory<std::string, Space> SpaceFactory; // =)

}

