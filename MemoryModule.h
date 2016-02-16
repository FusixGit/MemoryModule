#ifndef __MEMORY_MODULE_HEADER
#define __MEMORY_MODULE_HEADER

#include "stdafx.h"
#include <windows.h>

typedef void *HMEMORYMODULE;
typedef void *HMEMORYRSRC;
typedef void *HCUSTOMMODULE;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	void* EntryPoint;
	wchar_t* CommandLine;
	bool InThread;
	void* _ModuleHandle;
}	MEMORYMODULE_START_ARGS;

typedef HCUSTOMMODULE (*CustomLoadLibraryFunc)(LPCSTR, void *);
typedef FARPROC (*CustomGetProcAddressFunc)(HCUSTOMMODULE, LPCSTR, void *);
typedef void (*CustomFreeLibraryFunc)(HCUSTOMMODULE, void *);

/**
 * Load EXE/DLL from memory location with the given size.
 *
 * All dependencies are resolved using default LoadLibrary/GetProcAddress
 * calls through the Windows API.
 */
HMEMORYMODULE MemoryLoadLibrary(const void *, size_t);

/**
 * Load EXE/DLL from memory location with the given size using custom dependency
 * resolvers.
 *
 * Dependencies will be resolved using passed callback methods.
 */
HMEMORYMODULE MemoryLoadLibraryEx(const void *, size_t,
    CustomLoadLibraryFunc,
    CustomGetProcAddressFunc,
    CustomFreeLibraryFunc,
    void *);

/**
 * Get address of exported method. Supports loading both by name and by
 * ordinal value.
 */
FARPROC MemoryGetProcAddress(HMEMORYMODULE, LPCSTR);

/**
 * Free previously loaded EXE/DLL.
 */
void MemoryFreeLibrary(HMEMORYMODULE);

/**
 * Execute entry point (EXE only). The entry point can only be executed
 * if the EXE has been loaded to the correct base address or it could
 * be relocated (i.e. relocation information have not been stripped by
 * the linker).
 *
 * Important: calling this function will not return, i.e. once the loaded
 * EXE finished running, the process will terminate.
 *
 * Returns a negative value if the entry point could not be executed.
 */
int MemoryCallEntryPoint(HMEMORYMODULE, MEMORYMODULE_START_ARGS*);


/**
 * Find the location of a resource with the specified type and name.
 */
HMEMORYRSRC MemoryFindResource(HMEMORYMODULE, LPCTSTR, LPCTSTR);

/**
 * Find the location of a resource with the specified type, name and language.
 */
HMEMORYRSRC MemoryFindResourceEx(HMEMORYMODULE, LPCTSTR, LPCTSTR, WORD);

/**
 * Get the size of the resource in bytes.
 */
DWORD MemorySizeofResource(HMEMORYMODULE, HMEMORYRSRC);

/**
 * Get a pointer to the contents of the resource.
 */
LPVOID MemoryLoadResource(HMEMORYMODULE, HMEMORYRSRC);

/**
 * Load a string resource.
 */
int MemoryLoadString(HMEMORYMODULE, UINT, LPTSTR, int);

/**
 * Load a string resource with a given language.
 */
int MemoryLoadStringEx(HMEMORYMODULE, UINT, LPTSTR, int, WORD);

/**
 * Default implementation of CustomLoadLibraryFunc that calls LoadLibraryA
 * internally to load an additional libary.
 *
 * This is the default as used by MemoryLoadLibrary.
 */
HCUSTOMMODULE MemoryDefaultLoadLibrary(LPCSTR, void *);

/**
 * Default implementation of CustomGetProcAddressFunc that calls GetProcAddress
 * internally to get the address of an exported function.
 *
 * This is the default as used by MemoryLoadLibrary.
 */
FARPROC MemoryDefaultGetProcAddress(HCUSTOMMODULE, LPCSTR, void *);

/**
 * Default implementation of CustomFreeLibraryFunc that calls FreeLibrary
 * internally to release an additional libary.
 *
 * This is the default as used by MemoryLoadLibrary.
 */
void MemoryDefaultFreeLibrary(HCUSTOMMODULE, void *);
void MemoryModuleExecuteCode(MEMORYMODULE_START_ARGS*);

#ifdef __cplusplus
}
#endif

#endif
