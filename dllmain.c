#include <stdio.h>
#include <windows.h>

#define SIZE 6

BOOL WINAPI fake_BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
typedef BOOL (WINAPI * pBitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD);

BYTE JMP[SIZE] = {0};
BYTE oldBytes[SIZE] = {0};
pBitBlt BitBlt_addr = NULL;
DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;

void install_hook() {
	printf("Installing hook.\n");
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3};
	memcpy(JMP, tempJMP, SIZE);
	DWORD JMPSize = ((DWORD)fake_BitBlt - (DWORD)BitBlt_addr - 5);

	VirtualProtect((LPVOID)BitBlt_addr, SIZE,
			PAGE_EXECUTE_READWRITE, &oldProtect);

	memcpy(oldBytes, BitBlt_addr, SIZE);
	memcpy(&JMP[1], &JMPSize, 4);
	memcpy(BitBlt_addr, JMP, SIZE);

	VirtualProtect((LPVOID)BitBlt_addr, SIZE, oldProtect, NULL);
}

BOOL WINAPI fake_BitBlt(HDC hdcDest,
		int nXDest,
		int nYDest,
		int nWidth,
		int nHeight,
		HDC hdcSrc,
		int nXSrc,
		int nYSrc,
		DWORD dwRop)
{
	printf("fake_BitBlt called.\n");
	if (SRCCOPY == dwRop) {
		return TRUE;
	} else {
		printf("Running original BitBlt.\n");
		VirtualProtect((LPVOID)BitBlt_addr, SIZE, newProtect, NULL);
		memcpy(BitBlt_addr, oldBytes, SIZE);
		BOOL retVal = BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
		memcpy(BitBlt_addr, JMP, SIZE);
		VirtualProtect((LPVOID)BitBlt_addr, SIZE, oldProtect, NULL);
		return retVal;
	}
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		BitBlt_addr = (pBitBlt) GetProcAddress(GetModuleHandle("gdi32.dll"), "BitBlt");
		if (NULL != BitBlt_addr)
			install_hook();
	} else if (dwReason == DLL_PROCESS_DETACH) {
		printf("Removing hook.");
		memcpy(BitBlt_addr, oldBytes, SIZE);
	}

	return TRUE;
}