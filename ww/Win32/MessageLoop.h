/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

#include "ww/API.h"
#include "ww/Signal.h"
#include "ww/Win32/Types.h"
#include "yasli/Pointers.h"

namespace Win32{

class MessageFilter : public yasli::RefCounter
{
public:
	// return true to prevent further message processing
    virtual bool filter(MSG* msg) = 0;
};

class MessageLoop : public SignalScope
{
public:
	MessageLoop();

	int run();

	static void processPendingMessages();

	void interruptDialogLoop();
	int runDialogLoop(HWND dialog);

	Signal<>& signalIdle() { return signalIdle_; }
	Signal<>& signalQuit() { return signalQuit_; }

	void quit();
	void quit(int code);
	void onQuit() { signalQuit().emit(); }

	void installFilter(MessageFilter* filter);
	void uninstallFilter(MessageFilter* filter);

	static MessageLoop& instance();
protected:
	static Signal<> signalIdle_;
	Signal<> signalQuit_;

	HWND dialog_;
	bool dialogLoopInterrupted_;

	typedef std::vector<yasli::SharedPtr<MessageFilter> > Filters;
	Filters filters_;
};

}

