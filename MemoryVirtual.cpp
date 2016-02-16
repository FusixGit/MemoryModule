
#include <Windows.h>
#include "MemoryModule.h"
#include "nt.h"

#define SIZE (4096 / sizeof(char*))

typedef struct
{
    void* headers;
    unsigned char *codeBase;
    void* modules;
    int numModules;
    BOOL initialized;
    BOOL isDLL;
    BOOL isRelocated;
    void* loadLibrary;
    void* getProcAddress;
    void* freeLibrary;
    void *userdata;
    void* exeEntry;
    DWORD pageSize;
	int *CustomArgc;
	char *** CustomArgv;
	int *CustomWArgc;
	wchar_t *** CustomWArgv;
} MEMORYMODULE, *PMEMORYMODULE;

HMODULE hVirtualModule = NULL;
char sVirtualModuleName[128];

char* strndup(char const* name, size_t len)
{
   char *s = (char*)malloc(len + 1);
   if (s != NULL)
   {
      memcpy(s, name, len);
      s[len] = 0;
   }
   return s;
}

wchar_t* wcsndup(wchar_t* name, size_t len)
{
   wchar_t *s = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
   if (s != NULL)
   {
      memcpy(s, name, len*sizeof(wchar_t));
      s[len] = 0;
   }
   return s;
}

int aadd(char* name)
{
   char** _new;
   if ((__argc % SIZE) == 0)
   {
      if (__argv == NULL)
         _new = (char**)malloc(sizeof(char*) * (1 + SIZE));
      else
         _new = (char**)realloc(__argv, sizeof(char*) * (__argc + 1 + SIZE));
      if (_new == NULL)
         return -1;
      __argv = _new;
   }
   __argv[__argc++] = name;
   __argv[__argc] = NULL;
   return 0;
}

int wadd(wchar_t* name)
{
   wchar_t** _new;
   if ((__argc % SIZE) == 0)
   {
      if (__wargv == NULL)
         _new = (wchar_t**)malloc(sizeof(wchar_t*) * (1 + SIZE));
      else
         _new = (wchar_t**)realloc(__wargv, sizeof(wchar_t*) * (__argc + 1 + SIZE));
      if (_new == NULL)
         return -1;
      __wargv = _new;
   }
   __wargv[__argc++] = name;
   __wargv[__argc] = NULL;
   return 0;
}

int aexpand(char* name, int expand_wildcards)
{
   char* s;
   WIN32_FIND_DATAA fd;
   HANDLE hFile;
   BOOLEAN first = TRUE;
   char buffer[256];
   uintptr_t pos;

   if (expand_wildcards && (s = strpbrk(name, "*?")))
   {
      hFile = FindFirstFileA(name, &fd);
      if (hFile != INVALID_HANDLE_VALUE)
      {
         while(s != name && *s != '/' && *s != '\\')
            s--;
         pos = s - name;
         if (*s == '/' || *s == '\\')
            pos++;
         strncpy(buffer, name, pos);
         do
         {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
               strcpy(&buffer[pos], fd.cFileName);
               if (aadd(_strdup(buffer)) < 0)
               {
                  FindClose(hFile);
                  return -1;
               }
               first = FALSE;
            }
         }
         while(FindNextFileA(hFile, &fd));
         FindClose(hFile);
      }
   }
   if (first)
   {
      if (aadd(name) < 0)
         return -1;
   }
   else
      free(name);
   return 0;
}

int wexpand(wchar_t* name, int expand_wildcards)
{
   wchar_t* s;
   WIN32_FIND_DATAW fd;
   HANDLE hFile;
   BOOLEAN first = TRUE;
   wchar_t buffer[256];
   uintptr_t pos;

   if (expand_wildcards && (s = wcspbrk(name, L"*?")))
   {
      hFile = FindFirstFileW(name, &fd);
      if (hFile != INVALID_HANDLE_VALUE)
      {
         while(s != name && *s != L'/' && *s != L'\\')
            s--;
         pos = s - name;
         if (*s == L'/' || *s == L'\\')
            pos++;
         wcsncpy(buffer, name, pos);
         do
         {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
               wcscpy(&buffer[pos], fd.cFileName);
               if (wadd(_wcsdup(buffer)) < 0)
               {
                  FindClose(hFile);
                  return -1;
               }
               first = FALSE;
            }
         }
         while(FindNextFileW(hFile, &fd));
         FindClose(hFile);
      }
   }
   if (first)
   {
      if (wadd(name) < 0)
         return -1;
   }
   else
      free(name);
   return 0;
}

void __getmainargs(int* argc, char*** argv, char*** env, int expand_wildcards, int* new_mode, char* _acmdln)
{
   int i, afterlastspace, ignorespace, doexpand;
   size_t len;
   char* aNewCmdln;

   /* missing threading init */

   i = 0;
   afterlastspace = 0;
   ignorespace = 0;
   doexpand = expand_wildcards;

   if (__argv && _environ)
   {
      *argv = __argv;
      *env = _environ;
      *argc = __argc;
      return;
   }

   __argc = 0;

   len = strlen(_acmdln);

   /* Allocate a temporary buffer to be used instead of the original _acmdln parameter. */
   aNewCmdln = strndup(_acmdln, len);

   while (aNewCmdln[i])
   {
      if (aNewCmdln[i] == '"')
      {
         if(ignorespace)
         {
            ignorespace = 0;
         }
         else
         {
            ignorespace = 1;
            doexpand = 0;
         }
         memmove(aNewCmdln + i, aNewCmdln + i + 1, len - i);
         len--;
         continue;
      }

      if (aNewCmdln[i] == ' ' && !ignorespace)
      {
         aexpand(strndup(aNewCmdln + afterlastspace, i - afterlastspace), doexpand);
         i++;
         while (aNewCmdln[i] == ' ')
            i++;
         afterlastspace=i;
         doexpand = expand_wildcards;
      }
      else
      {
         i++;
      }
   }

   if (aNewCmdln[afterlastspace] != 0)
   {
      aexpand(strndup(aNewCmdln + afterlastspace, i - afterlastspace), doexpand);
   }

   /* Free the temporary buffer. */
   free(aNewCmdln);

   HeapValidate(GetProcessHeap(), 0, NULL);

   *argc = __argc;
   if (__argv == NULL)
   {
       __argv = (char**)malloc(sizeof(char*));
       __argv[0] = 0;
   }
   *argv = __argv;
   *env  = _environ;
   _pgmptr = _strdup(__argv[0]);
}

void __wgetmainargs(int* argc, wchar_t*** wargv, wchar_t*** wenv,
                    int expand_wildcards, int* new_mode, wchar_t*_wcmdln)
{
   int i, afterlastspace, ignorespace, doexpand;
   size_t len;
   wchar_t* wNewCmdln;

   /* missing threading init */

   i = 0;
   afterlastspace = 0;
   ignorespace = 0;
   doexpand = expand_wildcards;

   if (__wargv && _wenviron)
   {
      *wargv = __wargv;
      *wenv = _wenviron;
      *argc = __argc;
      return;
   }

   __argc = 0;

   len = wcslen(_wcmdln);

   /* Allocate a temporary buffer to be used instead of the original _wcmdln parameter. */
   wNewCmdln = wcsndup(_wcmdln, len);

   while (wNewCmdln[i])
   {
      if (wNewCmdln[i] == L'"')
      {
         if(ignorespace)
         {
            ignorespace = 0;
         }
         else
         {
            ignorespace = 1;
            doexpand = 0;
         }
         memmove(wNewCmdln + i, wNewCmdln + i + 1, (len - i) * sizeof(wchar_t));
         len--;
         continue;
      }

      if (wNewCmdln[i] == L' ' && !ignorespace)
      {
         wexpand(wcsndup(wNewCmdln + afterlastspace, i - afterlastspace), doexpand);
         i++;
         while (wNewCmdln[i] == L' ')
            i++;
         afterlastspace=i;
         doexpand = expand_wildcards;
      }
      else
      {
         i++;
      }
   }

   if (wNewCmdln[afterlastspace] != 0)
   {
      wexpand(wcsndup(wNewCmdln + afterlastspace, i - afterlastspace), doexpand);
   }

   /* Free the temporary buffer. */
   free(wNewCmdln);

   HeapValidate(GetProcessHeap(), 0, NULL);

   *argc = __argc;
   if (__wargv == NULL)
   {
       __wargv = (wchar_t**)malloc(sizeof(wchar_t*));
       __wargv[0] = 0;
   }
   *wargv = __wargv;
   *wenv = _wenviron;
   _wpgmptr = _wcsdup(__wargv[0]);
}

BOOL __stdcall Virtual_GetModuleHandleEx(DWORD dwFlags, void* pModuleName, HMODULE *hModuleRet)
{
	if(pModuleName == NULL)
	{
		*hModuleRet = NULL;
		return true;
	}
	
	return GetModuleHandleEx(dwFlags, (LPCWSTR)pModuleName, hModuleRet);
}

HMODULE __stdcall Virtual_GetModuleHandle(void* pModuleName)
{
	if(pModuleName == NULL)	return hVirtualModule;
	
	return GetModuleHandle((LPCWSTR)pModuleName);
}

char* __stdcall Virtual_GetCommandLineA()
{
	PEB32 *tPeb = (PEB32*)GetThreadPEB();
	RTL_USER_PROCESS_PARAMETERS_MEMMODULE *ProcessParams = (RTL_USER_PROCESS_PARAMETERS_MEMMODULE*)tPeb->ProcessParameters;
	if(ProcessParams->ExSign != 0xFFFFACAC)
	{
		return GetCommandLineA();
	}

	return GetCommandLineA();	//
}

wchar_t* __stdcall Virtual_GetCommandLineW()
{
	PEB32 *tPeb = (PEB32*)GetThreadPEB();
	RTL_USER_PROCESS_PARAMETERS_MEMMODULE *ProcessParams = (RTL_USER_PROCESS_PARAMETERS_MEMMODULE*)tPeb->ProcessParameters;
	if(ProcessParams->ExSign != 0xFFFFACAC)
	{
		return GetCommandLineW();
	}

	return GetCommandLineW(); //
}

int Virtual_getmainargs(int * _Argc, char *** _Argv, char *** _Env,  int _DoWildCard, void * _StartInfo)	// msvcrt
{
	PEB32 *tPeb = (PEB32*)GetThreadPEB();
	RTL_USER_PROCESS_PARAMETERS_MEMMODULE *ProcessParams = (RTL_USER_PROCESS_PARAMETERS_MEMMODULE*)tPeb->ProcessParameters;
	if(ProcessParams->ExSign != 0xFFFFACAC)
	{
		__getmainargs(_Argc, _Argv, _Env, _DoWildCard, (int*)_StartInfo, GetCommandLineA());
		return 0;
	}

	char* cLine = new char[ProcessParams->CommandLine.MaximumLength];
	memset(cLine, 0x00, ProcessParams->CommandLine.MaximumLength);
	WideCharToMultiByte(CP_UTF8, 0, ProcessParams->CommandLine.Buffer, lstrlenW(ProcessParams->CommandLine.Buffer), cLine, ProcessParams->CommandLine.MaximumLength, NULL, NULL);

	__getmainargs(_Argc, _Argv, _Env, _DoWildCard, (int*)_StartInfo, cLine);

	MEMORYMODULE *hMemoryModule = (MEMORYMODULE*)ProcessParams->MemModule;
	hMemoryModule->CustomArgc = (int*)_Argc;
	hMemoryModule->CustomArgv = (char***)_Argv;

	return 0;
}

int Virtual_wgetmainargs(int *_Argc, wchar_t ***_Argv, wchar_t ***_Env, int _DoWildCard, void * _StartInfo)	// msvcrt
{	
	PEB32 *tPeb = (PEB32*)GetThreadPEB();
	RTL_USER_PROCESS_PARAMETERS_MEMMODULE *ProcessParams = (RTL_USER_PROCESS_PARAMETERS_MEMMODULE*)tPeb->ProcessParameters;
	if(ProcessParams->ExSign != 0xFFFFACAC)
	{
		__wgetmainargs(_Argc, _Argv, _Env, _DoWildCard, (int*)_StartInfo, GetCommandLineW());
		return 0;
	}
	__wgetmainargs(_Argc, _Argv, _Env, _DoWildCard, (int*)_StartInfo, ProcessParams->CommandLine.Buffer);
	
	MEMORYMODULE *hMemoryModule = (MEMORYMODULE*)ProcessParams->MemModule;
	hMemoryModule->CustomWArgc = (int*)_Argc;
	hMemoryModule->CustomWArgv = (wchar_t***)_Argv;
	return 0;
}
