#include "stdafx.h"
#include <float.h>
#include <stdio.h>
#include "BugslayerUtil.h"

class XErrorHandler
{
public:
	 XErrorHandler();

	void	 Abort(const char* message, const char* subj = NULL);
	void	 WriteLog(char* error, char* msg);
};

char exceptMSG[512] 	= "EXCEPTION OCCURED";

void setExceptionMessage(const char* msg){ strncpy(exceptMSG, msg, 511); }

#pragma warning (disable : 4073)
#pragma init_seg(lib)
XErrorHandler ErrH;

static char _ConvertBuffer[64];

char* dwtoa(unsigned int a)
{
	int len = 8 - strlen(ultoa(a,_ConvertBuffer,16));
	for(int i = 0;i < len;i++)
		_ConvertBuffer[i] = '0';
	strlen(ultoa(a,_ConvertBuffer + len,16));
	return _ConvertBuffer;
}
char* wtoa(unsigned int a)
{
	int len = 4 - strlen(ultoa(a,_ConvertBuffer,16));
	for(int i = 0;i < len;i++)
		_ConvertBuffer[i] = '0';
	strlen(ultoa(a,_ConvertBuffer + len,16));
	return _ConvertBuffer;
}
char* uctoa(unsigned char a)
{
	int len = 2 - strlen(ultoa(a,_ConvertBuffer,16));
	if(len == 1)
		_ConvertBuffer[0] = '0';
	strlen(ultoa(a,_ConvertBuffer + len,16));
	return _ConvertBuffer;
}

LONG APIENTRY exHandler(EXCEPTION_POINTERS *except_info)
{
	static bool handled = false;
	if(handled)
		return EXCEPTION_CONTINUE_EXECUTION;
	handled = true;

	_clearfp();
	SetUnhandledExceptionFilter(NULL);

	static char msg[100000];

	strcpy(msg, "");

	switch(except_info->ExceptionRecord->ExceptionCode)
	{
	 case EXCEPTION_INT_DIVIDE_BY_ZERO:
		 strcat(msg, "INTEGER DIVIDE ERROR");
		 break;
	 case EXCEPTION_INT_OVERFLOW:
		 strcat(msg, "INTEGER OVERFLOW");
		 break;
	 case EXCEPTION_ACCESS_VIOLATION:
		 strcat(msg, "MEMORY ACCESS VIOLATION");
		 break;
	 case EXCEPTION_BREAKPOINT:
		 strcat(msg, "A BREAKPOINT WAS ENCOUNTERED");
		 break;
	 case EXCEPTION_DATATYPE_MISALIGNMENT:
		 strcat(msg, "DATA TYPE MISALIGMENT");
		 break;
	 case EXCEPTION_SINGLE_STEP:
		 strcat(msg, "TRACE TRAP");
		 break;
	 case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		 strcat(msg, "OUT OF BOUNDS FOR ARRAY");
		 break;
	 case EXCEPTION_FLT_DENORMAL_OPERAND:
		 strcat(msg, "FLOATING POINT DENORMAL OPERAND");
		 break;
	 case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		 strcat(msg, "FLOATING POINT DIVIDE BY ZERO");
		 break;
	 case EXCEPTION_FLT_INEXACT_RESULT:
		 strcat(msg, "FLOATING POINT INEXACT RESULT");
		 break;
	 case EXCEPTION_FLT_INVALID_OPERATION:
		 strcat(msg, "FLOATING POINT INVALID OPERATION");
		 break;
	 case EXCEPTION_FLT_OVERFLOW:
		 strcat(msg, "FLOATING POINT OVERFLOW");
		 break;
	 case EXCEPTION_FLT_STACK_CHECK:
		 strcat(msg, "FLOATING POINT STACK CHECK FAILED");
		 break;
	 case EXCEPTION_FLT_UNDERFLOW:
		 strcat(msg, "FLOATING POINT UNDERFLOW");
		 break;
	 case EXCEPTION_PRIV_INSTRUCTION:
		 strcat(msg, "PRIVELEGED INSTRUCTION");
		 break;
	 default:
		 strcat(msg, "UNKNOWN ERROR TYPE");
	}
	strcat(msg," AT LOCATION 0x");
	strcat(msg,strupr(itoa((int)except_info->ExceptionRecord->ExceptionAddress,_ConvertBuffer,16)));

	static int attempt_to_show_context = 0;
	if(!attempt_to_show_context){
		PCONTEXT p = except_info -> ContextRecord;
		if((p -> ContextFlags & CONTEXT_INTEGER) && (p -> ContextFlags & CONTEXT_CONTROL) &&
			(p -> ContextFlags & CONTEXT_CONTROL)){
			attempt_to_show_context = 1;
			strcat(msg,"\r\n\r\nRegisters:\r\n");
			strcat(msg,"EAX="); strcat(msg, dwtoa(p -> Eax));
			strcat(msg,"  CS="); strcat(msg, wtoa(p -> SegCs));
			strcat(msg,"  EIP="); strcat(msg, dwtoa(p -> Eip));
			strcat(msg,"  EFLAGS="); strcat(msg, dwtoa(p -> EFlags));

			strcat(msg,"\r\nEBX="); strcat(msg, dwtoa(p -> Ebx));
			strcat(msg,"  SS="); strcat(msg, wtoa(p -> SegSs));
			strcat(msg,"  ESP="); strcat(msg, dwtoa(p -> Esp));
			strcat(msg,"  EBP="); strcat(msg, dwtoa(p -> Ebp));

			strcat(msg,"\r\nECX="); strcat(msg, dwtoa(p -> Ecx));
			strcat(msg,"  DS="); strcat(msg, wtoa(p -> SegDs));
			strcat(msg,"  ESI="); strcat(msg, dwtoa(p -> Esi));
			strcat(msg,"  FS="); strcat(msg, wtoa(p -> SegFs));

			strcat(msg,"\r\nEDX="); strcat(msg, dwtoa(p -> Edx));
			strcat(msg,"  ES="); strcat(msg, wtoa(p -> SegEs));
			strcat(msg,"  EDI="); strcat(msg, dwtoa(p -> Edi));
			strcat(msg,"  GS="); strcat(msg, wtoa(p -> SegGs));
			
			strcat(msg,"\r\n\r\nBytes at CS::EIP:\r\n");
			unsigned char* code = (unsigned char*)(p -> Eip);
			int i;
			for(i = 0;i < 16;i++){
				strcat(msg, uctoa(code[i]));
				strcat(msg," "); 
				}

			strcat(msg,"\r\n\r\nStack dump:\r\n");
			unsigned int* stack = (unsigned int*)(p -> Esp);
			for(i = 0;i < 32;i++){
				strcat(msg, dwtoa(stack[i]));
				strcat(msg, (i & 7) == 7 ? "\r\n" : " "); 
				}

			strcat(msg,"\r\nCall stack:\r\n");
			int dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
			const char * szBuff = GetFirstStackTraceString ( dwOpts, except_info) ;
			int iterations = 100;
			while(szBuff && --iterations) {
				strcat(msg, szBuff);
				strcat(msg,"\r\n");
				szBuff = GetNextStackTraceString ( dwOpts , except_info ) ;
				}
			}
		}


	ErrH.WriteLog(exceptMSG, msg);
	ErrH.Abort(exceptMSG, msg);
	return EXCEPTION_EXECUTE_HANDLER;
}

XErrorHandler::XErrorHandler()
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exHandler);
}

void XErrorHandler::Abort(const char* message, const char* subj)
{
	static char outmsg[10000]="";
	UINT attr=MB_OK;

	strcpy(outmsg,message);
	if(subj){
		strcat(outmsg,"\r");
		strcat(outmsg,subj);
	}

	MessageBox(NULL,outmsg,exceptMSG,attr | MB_TOPMOST | MB_SYSTEMMODAL);
	exit(0);
}

void XErrorHandler::WriteLog(char* error, char* msg)
{
    FILE* f = fopen("Error.log", "wb");
    if(!f)
        return;

    fwrite(msg, 1, strlen(msg), f);
    fclose(f);
}

