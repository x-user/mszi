#ifndef __INJECTOR_H__
#define __INJECTOR_H__

#include <stdio.h>
#include <windows.h>

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
	LPVOID pLoadLibraryA;
	CHAR LibraryPath[MAX_PATH];
} InjectStruct, *pInjectStruct;
#pragma pack()

/**
 * Get rebased pointer to struct member.
 * @param ptr Pointer to struct member.
 * @return Pointer to struct member in remote process.
 */
DWORD rebasePtr(PVOID ptr);

/**
 * Inject code in target process.
 * @param dwProcId The identifier of the target process.
 * @return TRUE if success, and FALSE otherwise.
 */
BOOL injectFunct(DWORD dwProcId);

#endif // __INJECTOR_H__