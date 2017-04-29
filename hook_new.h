#ifndef __HOOK_NEW_H__
#define __HOOK_NEW_H__

#include "error.h"

#define JMP			0xE9
#define JMP_BACK	0xF9EB
#define LONG_NOP	0xFF8B

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
 * Remove hook
 */
BOOL HotUnpatch(void * procBase);

/**
 * BitBlt pointer typedef
 */
typedef BOOL (WINAPI * pBitBlt)(HDC,int,int,int,int,HDC,int,int,DWORD);

#endif // __HOOK_NEW_H__