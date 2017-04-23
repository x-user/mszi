#ifndef __HOOK_H__
#define __HOOK_H__

#include <stdio.h>
#include <windows.h>

#define SIZE 6

BOOL WINAPI fake_BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
typedef BOOL (WINAPI * pBitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD);

/*
 * get needed information and install hook first time
 */
void install_hook();

/**
 * BitBlt hook function
 */
BOOL WINAPI fake_BitBlt(HDC hdcDest, int, int, int, int, HDC, int, int, DWORD);

#endif // __HOOK_H__