#include "hook.h"

BYTE JMP[SIZE] = {0};
BYTE oldBytes[SIZE] = {0};
pBitBlt BitBlt_addr = NULL;

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved) {
	BOOL result = FALSE;

	// get handle of gdi32.dll
	HANDLE hGdi = GetModuleHandleA("gdi32.dll");
	if (hGdi) {
		// get BitBlt base address
		BitBlt_addr = GetProcAddress(hGdi, "BitBlt");
		if (BitBlt_addr) {
			switch (dwReason) {
			case DLL_PROCESS_ATTACH:
				result = install_hook();

				break;
			case DLL_PROCESS_DETACH:
				result = uninstall_hook();

				break;
			}
		}

		CloseHandle(hGdi);
	}

	return result;
}

BOOL install_hook() {
	DWORD oldProtect;
	BOOL result = FALSE;

	BYTE tempJMP[SIZE] = { 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3 };
	// copy trampoline code
	memcpy(JMP, tempJMP, SIZE);
	// calculate address
	DWORD JMPSize = ((DWORD) fake_BitBlt - (DWORD) BitBlt_addr - 5);

	// get write access
	if (VirtualProtect((LPVOID) BitBlt_addr, SIZE,
					   PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		// copy original bytes
		memcpy(oldBytes, BitBlt_addr, SIZE);
		// replace nops in trampoline with address
		memcpy(&JMP[1], &JMPSize, 4);
		// splice BitBlt function
		memcpy(BitBlt_addr, JMP, SIZE);

		result = TRUE;

		// restore protection
		VirtualProtect((LPVOID)BitBlt_addr, SIZE, oldProtect, &oldProtect);
	}

	return result;
}

BOOL uninstall_hook() {
	DWORD oldProtect;
	BOOL result = FALSE;

	if (VirtualProtect((LPVOID)BitBlt_addr, SIZE,
					   PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		memcpy(BitBlt_addr, oldBytes, SIZE);

		result = TRUE;

		VirtualProtect((LPVOID)BitBlt_addr, SIZE, oldProtect, &oldProtect);
	}

	return result;
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
	BOOL result = FALSE;

	if (SRCCOPY == dwRop) {
		SetLastError(ERROR_ACCESS_DENIED);
	}
	else {
		DWORD oldProtect;

		// get write access
		if (VirtualProtect((LPVOID)BitBlt_addr, SIZE,
						   PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			// restore original code
			memcpy(BitBlt_addr, oldBytes, SIZE);

			// call original BitBlt function
			result = BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);

			// restore hook
			memcpy(BitBlt_addr, JMP, SIZE);

			// restore protection
			VirtualProtect((LPVOID)BitBlt_addr, SIZE, oldProtect, &oldProtect);
		}
	}

	return result;
}
