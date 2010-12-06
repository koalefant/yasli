#pragma once

#include "ww/API.h"
#include "yasli/sigslot.h"
#include "yasli/Pointers.h"

namespace Win32{
    class MessageLoop;
};

namespace ww{

class WW_API Application : public sigslot::has_slots
{
public:
    Application(void* instance = 0);
    ~Application();

    int run();
    void quit();
    void quit(int returnCode);

    sigslot::signal0& signalIdle() { return signalIdle_; }
    sigslot::signal0& signalQuit() { return signalQuit_; }

	void processPendingMessages();
    Win32::MessageLoop* _messageLoop(){ return messageLoop_.get(); }
private:
    void onLoopIdle();
    void onLoopQuit();

    sigslot::signal0 signalIdle_;
    sigslot::signal0 signalQuit_;

    AutoPtr<Win32::MessageLoop> messageLoop_;
};

};

