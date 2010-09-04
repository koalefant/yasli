#include "StdAfx.h"
#include "ww/Win32/MessageLoop.h"

#include <algorithm>
#include <windows.h>
#include <windowsx.h>
#include <CrtDbg.h>

namespace Win32{

sigslot::signal0 MessageLoop::signalIdle_;

int MessageLoop::run()
{
    MSG msg;
    while(true){
        if(!PeekMessage(&msg, 0, 0, 0, TRUE)){
            signalIdle_.emit();
            Sleep(10);
        }
        else{
            if(msg.message == WM_QUIT)
                return int(msg.wParam);

            TranslateMessage(&msg); // генерит WM_CHAR из WM_KEYDOWN и т.п.

            bool filtered = false;
            for(Filters::iterator it = filters_.begin(); it != filters_.end(); ++it)
                if(it->get()->filter(&msg))
                    filtered = true;

            if(!filtered)
                DispatchMessage(&msg);
            profiler_quant();
        }
    }
    return 0;
}

void MessageLoop::quit()
{
    quit(0);
}

void MessageLoop::quit(int code)
{
    onQuit();
    ::PostQuitMessage(code);
}

void MessageLoop::installFilter(MessageFilter* filter)
{
    filters_.push_back(filter);
}

void MessageLoop::uninstallFilter(MessageFilter* filter)
{
    Filters::iterator it;
    for(it = filters_.begin(); it != filters_.end(); )
        if(it->get() == filter)
            it = filters_.erase(it);
        else
            ++it;
}


MessageLoop& MessageLoop::instance()
{
    static MessageLoop messageLoop;
    return messageLoop;
}

}
