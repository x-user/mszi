#ifndef __HOOK_H__
#define __HOOK_H__

#include <stdio.h>
#include <windows.h>

#define SIZE 6

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
 * 1st hook install
 */
BOOL install_hook();

/**
 * final hook deletion
 */
BOOL uninstall_hook();

/**
 * BitBlt pointer typedef
 */
typedef BOOL (WINAPI * pBitBlt)(HDC,int,int,int,int,HDC,int,int,DWORD);

#endif // __HOOK_H__
