#include <Windows.h>

#pragma pack(push)
#pragma pack(1)

typedef struct _LSA_UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
  BYTE           Reserved1[16];
  PVOID          Reserved2[10];
  UNICODE_STRING ImagePathName;
  UNICODE_STRING CommandLine;
	char Reserved[1024];
} RTL_USER_PROCESS_PARAMETERS;

typedef struct _RTL_USER_PROCESS_PARAMETERS_MEMMODULE {
  BYTE           Reserved1[16];
  PVOID          Reserved2[10];
  UNICODE_STRING ImagePathName;
  UNICODE_STRING CommandLine;
  char Reserved[1024];
  DWORD			 ExSign;		// 0cFFFFACAC;
  PVOID			 MemModule;
} RTL_USER_PROCESS_PARAMETERS_MEMMODULE;


typedef struct _PEB 
{
  BYTE                          Reserved1[2];
  BYTE                          BeingDebugged;
  BYTE                          Reserved2[1];
  PVOID                         Reserved3[2];
	  void* /*PPEB_LDR_DATA*/                 Ldr;
  RTL_USER_PROCESS_PARAMETERS*  ProcessParameters;
  BYTE                          Reserved4[104];
  PVOID                         Reserved5[52];
  void* /*PPS_POST_PROCESS_INIT_ROUTINE*/ PostProcessInitRoutine;
  BYTE                          Reserved6[128];
  PVOID                         Reserved7[1];
  ULONG                         SessionId;
} PEB32;

typedef struct  
{
	void *Buf;
	DWORD Count;
} COMMANDLINE_ARGS;

#pragma pack(pop)

extern "C"
{
	__inline void* GetThreadTIB();
	__inline void* GetThreadTEB();
	__inline void* GetThreadPEB();
}
