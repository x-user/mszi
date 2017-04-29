#ifndef __INJECTOR_H__
#define __INJECTOR_H__

#include "error.h"

// ASM commands.
#define PUSH			0x68
#define NOP				0x90
#define CALL_DWORD_PTR	0x15FF

#pragma pack(1)
/**
 * structure for code injection
 */
typedef struct tInjectStruct {
	// code
	BYTE cmd0;
	BYTE cmd1;
	DWORD cmd1ar;
	WORD cmd2;
	DWORD cmd2ar;
	BYTE cmd3;
	DWORD cmd3ar;
	WORD cmd4;
	DWORD cmd4ar;
	// data
	LPVOID pExitThread;
	LPVOID pLoadLibraryW;
	WCHAR LibraryPath[MAX_PATH];
} InjectStruct, *pInjectStruct;
#pragma pack()

/**
 * Inject code in target process.
 */
BOOL injectFunct(LPWSTR lpHookDll);

/**
 * Get rebased pointer to struct member.
 */
DWORD rebasePtr(PVOID ptr);

#endif // __INJECTOR_H__