#pragma once

#include "ww/API.h"
#include "ww/sigslot.h"
#include "yasli/Pointers.h"

namespace Win32{
    class MessageLoop;
};

namespace ww{

class WW_API Application : public has_slots
{
public:
    Application(void* hInstance = 0);
    ~Application();

	static Application* get();

    int run();
    void quit();
    void quit(int returnCode);

    signal0& signalIdle() { return signalIdle_; }
    signal0& signalQuit() { return signalQuit_; }

	void processPendingMessages();
    Win32::MessageLoop* _messageLoop(){ return messageLoop_.get(); }
private:
    void onLoopIdle();
    void onLoopQuit();

    signal0 signalIdle_;
    signal0 signalQuit_;

    AutoPtr<Win32::MessageLoop> messageLoop_;

	static Application* globalInstance_;
};

};

