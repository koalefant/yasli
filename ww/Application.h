/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/API.h"
#include "ww/Signal.h"
#include "yasli/Pointers.h"

namespace Win32{
    class MessageLoop;
};

namespace ww{

class Application : public SignalScope
{
public:
    Application(void* hInstance = 0);
    ~Application();

	static Application* get();

    int run();
    void quit();
    void quit(int returnCode);

    Signal<>& signalIdle() { return signalIdle_; }
    Signal<>& signalQuit() { return signalQuit_; }

	void processPendingMessages();
    Win32::MessageLoop* _messageLoop(){ return messageLoop_.get(); }
private:
    void onLoopIdle();
    void onLoopQuit();

    Signal<> signalIdle_;
    Signal<> signalQuit_;

    AutoPtr<Win32::MessageLoop> messageLoop_;

	static Application* globalInstance_;
};

};

