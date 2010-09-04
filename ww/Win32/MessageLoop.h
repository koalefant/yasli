#pragma once

#include "ww/API.h"
#include "yasli/sigslot.h"
#include "yasli/Pointers.h"
#include "ww/Win32/Types.h"

namespace Win32{

class WW_API MessageFilter : public yasli::RefCounter
{
public:
    virtual bool filter(MSG* msg) = 0; // возвращаем true чтобы заблокировать обработку сообщения
};

class WW_API MessageLoop : public sigslot::has_slots
{
public:
    int run();

    sigslot::signal0& signalIdle() { return signalIdle_; }
    sigslot::signal0& signalQuit() { return signalQuit_; }

    void quit();
    void quit(int code);
    void onQuit() { signalQuit().emit(); }

    void installFilter(MessageFilter* filter);
    void uninstallFilter(MessageFilter* filter);
    
    static MessageLoop& instance();
protected:
    static sigslot::signal0 signalIdle_;
    sigslot::signal0 signalQuit_;

    typedef std::vector<yasli::SharedPtr<MessageFilter> > Filters;
    Filters filters_;
};

}

