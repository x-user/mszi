#include "hook_new.h"

pBitBlt origBitBlt = NULL;

// hotpachable block patterns
BYTE pattern1[7] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x8B, 0xFF };
BYTE pattern2[7] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0xFF };

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
	// get BitBlt base address
	pBitBlt BitBlt_base =
		GetProcAddress(GetModuleHandle("gdi32.dll"), "BitBlt");

	switch (dwReason) {
	case DLL_PROCESS_DETACH:
		// remove hook
		if (BitBlt_base) {
			printf("Attempting to uninstall BitBlt hook..");
			if (!HotUnpatch(BitBlt_base) {
				printLastError();
			}
			else {
				printf("success\n");
			}
		}

		break;
	case DLL_PROCESS_ATTACH:
		// install hook
		if (BitBlt_base) {
			printf("Attempting to install BitBlt hook..");
			if (!HotPatch(BitBlt_base, &fake_BitBlt, (void**) &origBitBlt)) {
				printLastError();
			}
			else {
				printf("success\n");
			}
		}

		break;
	}

	return TRUE;
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
	else if (origBitBlt) {
		return origBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	}
}

BOOL HotPatch(void * procBase, void * hookBase, void ** ppOrigFn) {
	DWORD oldProtect;
	BOOL result = FALSE;

	// get patchBase address
	BYTE * patchBase = (BYTE*) procBase - 5;
	// calculate long jump offset
	DWORD jmpOffset = (DWORD) hookBase - (DWORD) procBase - 5;

	// check if function hotpatchable
	if (0 != memcmp(patchBase, &pattern1, 7)
		&& 0 != memcmp(patchBase, &pattern2, 7))
	{
		// not hotpachable function
		SetLastError(ERROR_INVALID_FUNCTION);
	}
	else {
		// get write access
		if (VirtualProtect(patchBase, 7,
						   PAGE_EXECUTE_WRITECOPY, &oldProtect))
		{
			// write long jump opcode
			*patchBase = JMP;
			// write long jump offset
			memcpy(patchBase + 1, &jmpOffset, 4);
			// write back jump
			*((WORD*) procBase) = JMP_BACK;

			result = TRUE;

			// restore protection
			VirtualProtect(patchBase, 7, oldProtect, &oldProtect);

			// store address to call original function code
			if (ppOrigFn)
				*ppOrigFn = (BYTE*) procBase + 2;
		}
	}

	return result;
}

BOOL HotUnpatch(void * procBase) {
	DWORD oldProtect;
	BOOL result = FALSE;

	if (JMP_BACK != *((WORD*) procBase)) {
		// not hotpatched function
		SetLastError(ERROR_INVALID_FUNCTION);
	}
	else {
		// get patchBase address
		BYTE * patchBase = (BYTE*) procBase - 5;

		// get write access
		if (VirtualProtect(patchBase, 7,
						   PAGE_EXECUTE_WRITECOPY, &oldProtect))
		{
			// remove back jump
			*((WORD*) procBase) = LONG_NOP;
			// remove long jump
			memset(patchBase, 0x90, 5);

			result = TRUE;

			// restore protection
			VirtualProtect(patchBase, 7, oldProtect, &oldProtect);
		}
	}

	return result;
}