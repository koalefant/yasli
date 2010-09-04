#include "StdAfx.h"
#include "ww/Application.h"
#include "ww/Win32/Types.h"
#include "ww/Win32/MessageLoop.h"
#include "ww/Win32/Window.h"

namespace ww{

Application::Application(void* instance)
: messageLoop_( new Win32::MessageLoop() )
{
    messageLoop_->signalIdle().connect(this, &Application::onLoopIdle);
    messageLoop_->signalQuit().connect(this, &Application::onLoopQuit);

#ifndef WW_DLL
    if(instance)
        Win32::_setGlobalInstance((HINSTANCE)instance);
#endif
}

Application::~Application()
{
}

int Application::run()
{
    ESCAPE(messageLoop_, return 0);
    return messageLoop_->run();
}

void Application::quit()
{
    quit(0);
}

void Application::quit(int returnCode)
{
    ESCAPE(messageLoop_, return);
    messageLoop_->quit(returnCode);
}

void Application::onLoopIdle()
{
    signalIdle_.emit();
}

void Application::onLoopQuit()
{
    signalQuit_.emit();
}

}


#ifdef WW_DLL
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
   UNREFERENCED_PARAMETER(lpReserved);

   if(dwReason == DLL_PROCESS_ATTACH){
       // загрузка DLL
       Win32::_setGlobalInstance(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH){
       // выгрузка DLL
   }
   return TRUE;
}
#endif
