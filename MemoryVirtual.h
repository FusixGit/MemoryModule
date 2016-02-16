#include "stdafx.h"
#include <windows.h>

BOOL	__stdcall	Virtual_GetModuleHandleEx(DWORD, void* pModuleName, HMODULE *hModuleRet);
HMODULE __stdcall	Virtual_GetModuleHandle(void* pModuleName);
char*	__stdcall	Virtual_GetCommandLineA();
wchar_t* __stdcall  Virtual_GetCommandLineW();

int Virtual_getmainargs(int *_Argc, char ***_Argv, char ***_Env, int _DoWildCard, void * _StartInfo);
int Virtual_wgetmainargs(int *_Argc, wchar_t ***_Argv, wchar_t ***_Env, int _DoWildCard, void * _StartInfo);
