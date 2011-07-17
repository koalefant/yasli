#pragma once

#include "ww/API.h"
#include "ww/sigslot.h"
#include "ww/Win32/Types.h"
#include "yasli/Pointers.h"

namespace Win32{

class WW_API MessageFilter : public yasli::RefCounter
{
public:
    virtual bool filter(MSG* msg) = 0; // возвращаем true чтобы заблокировать обработку сообщения
};

class WW_API MessageLoop : public sigslot::has_slots
{
public:
	MessageLoop();

	int run();

	static void processPendingMessages();

	void interruptDialogLoop();
	int runDialogLoop(HWND dialog);

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

	HWND dialog_;
	bool dialogLoopInterrupted_;

	typedef std::vector<yasli::SharedPtr<MessageFilter> > Filters;
	Filters filters_;
};

}

