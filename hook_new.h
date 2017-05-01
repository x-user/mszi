#ifndef __HOOK_NEW_H__
#define __HOOK_NEW_H__

#include "windows.h"

#define PATCH_SIZE		7
#define LONG_JMP_SIZE	5
#define SHORT_JMP_SIZE	2

#define LONG_JMP		0xE9
#define NOP				0x90
#define SHORT_JMP		0xF9EB
#define LONG_NOP		0xFF8B

#define PATTERN1	0x90909090
#define PATTERN2	0xCCCCCCCC

/**
 * BitBlt hook
 */
BOOL WINAPI fake_BitBlt(HDC hdcDest,
						int nXDest,
						int nYDest,
						int nWidth,
						int nHeight,
						HDC hdcSrc,
						int nXSrc,
						int nYSrc,
						DWORD dwRop
);

/**
 * Install hook
 */
BOOL HotPatch(void * procBase, void * hookBase, void ** ppOrigFn);

/**
 * Uninstall hook
 */
BOOL HotUnpatch(void * procBase);

/**
 * BitBlt pointer typedef
 */
typedef BOOL (WINAPI * pBitBlt)(HDC,int,int,int,int,HDC,int,int,DWORD);

#endif // __HOOK_NEW_H__
