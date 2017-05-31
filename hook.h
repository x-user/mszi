#ifndef __HOOK_H__
#define __HOOK_H__

#include <windows.h>

#define SIZE 6

/**
 * BitBlt hook function
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
 * Install BitBlt hook
 */
BOOL install_hook();

/**
 * Remove BitBlt hook
 */
BOOL remove_hook();

/**
 * BitBlt pointer typedef
 */
typedef BOOL (WINAPI * pBitBlt)(HDC,int,int,int,int,HDC,int,int,DWORD);

#endif // __HOOK_H__
