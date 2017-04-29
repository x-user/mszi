#include "hook.h"

BYTE JMP[SIZE] = {0};
BYTE oldBytes[SIZE] = {0};
pBitBlt BitBlt_addr = NULL;
DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		// store original BitBlt address
		BitBlt_addr = GetProcAddress(GetModuleHandle("gdi32.dll"), "BitBlt");

		if (NULL != BitBlt_addr) {
			install_hook();
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		// remove hook on library unload
		memcpy(BitBlt_addr, oldBytes, SIZE);
	}

	return TRUE;
}

void install_hook() {
	BYTE tempJMP[SIZE] = { 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3 };
	// copy trampoline code
	memcpy(JMP, tempJMP, SIZE);
	// calculate address
	DWORD JMPSize = ((DWORD)fake_BitBlt - (DWORD)BitBlt_addr - 5);

	// get write access
	VirtualProtect((LPVOID)BitBlt_addr, SIZE,
				   PAGE_EXECUTE_READWRITE, &oldProtect);

	// copy original bytes
	memcpy(oldBytes, BitBlt_addr, SIZE);
	// replace nops in trampoline with address
	memcpy(&JMP[1], &JMPSize, 4);
	// splice BitBlt function
	memcpy(BitBlt_addr, JMP, SIZE);

	// restore protection
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
	if (SRCCOPY == dwRop) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}
	else {
		// get write access
		VirtualProtect((LPVOID)BitBlt_addr, SIZE, newProtect, NULL);
		// restore original code
		memcpy(BitBlt_addr, oldBytes, SIZE);

		// call original BitBlt function
		BOOL retVal = BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);

		// restore hook
		memcpy(BitBlt_addr, JMP, SIZE);
		// restore protection
		VirtualProtect((LPVOID)BitBlt_addr, SIZE, oldProtect, NULL);

		return retVal;
	}
}
