#include "hook_new.h"

pBitBlt origBitBlt = NULL;

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    BOOL result = FALSE;

    // get handle of gdi32.dll
    HANDLE hGdi = GetModuleHandleA("gdi32.dll");
    if (hGdi)
    {
        // get BitBlt base address
        pBitBlt BitBlt_base = (pBitBlt) GetProcAddress(hGdi, "BitBlt");
        if (BitBlt_base)
        {
            switch (dwReason)
            {
                case DLL_PROCESS_ATTACH:
                    // install hook
                    result = HotPatch((void*) BitBlt_base,
                                      (void*) fake_BitBlt,
                                      (void**) &origBitBlt);

                    break;
                case DLL_PROCESS_DETACH:
                    // uninstall hook
                    result = HotUnpatch((void*) BitBlt_base);

                    break;
            }
        }

        CloseHandle(hGdi);
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
    if (SRCCOPY == dwRop)
    {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    else if (origBitBlt)
    {
        return origBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
    }
}

BOOL HotPatch(void * procBase, void * hookBase, void ** ppOrigFn)
{
    BOOL result = FALSE;
    // get patch address
    BYTE * patchBase = (BYTE*) procBase - LONG_JMP_SIZE;

    // check if function hotpatchable
    if (LONG_NOP != *((WORD*) procBase)
            || PATTERN1 != *((DWORD*) patchBase)
            && PATTERN2 != *((DWORD*) patchBase))
    {
        // not hotpachable function
        SetLastError(ERROR_INVALID_FUNCTION);
    }
    else
    {
        DWORD oldProtect;

        // get write access
        if (VirtualProtect(patchBase, PATCH_SIZE,
                           PAGE_EXECUTE_WRITECOPY, &oldProtect))
        {
            // write long jump opcode
            *patchBase = LONG_JMP;
            // write long jump offset
            *((DWORD*) procBase - 1) = (DWORD) hookBase - (DWORD) procBase;
            // write short jump
            *((WORD*) procBase) = SHORT_JMP;

            result = TRUE;

            // restore protection
            VirtualProtect(patchBase, PATCH_SIZE, oldProtect, &oldProtect);

            // store address of original function code
            if (ppOrigFn)
            {
                *ppOrigFn = (BYTE*) procBase + SHORT_JMP_SIZE;
            }
        }
    }

    return result;
}

BOOL HotUnpatch(void * procBase)
{
    BOOL result = FALSE;

    if (SHORT_JMP != *((WORD*) procBase))
    {
        // not hotpatched function
        SetLastError(ERROR_INVALID_FUNCTION);
    }
    else
    {
        DWORD oldProtect;
        // get patch address
        BYTE * patchBase = (BYTE*) procBase - LONG_JMP_SIZE;

        // get write access
        if (VirtualProtect(patchBase, PATCH_SIZE,
                           PAGE_EXECUTE_WRITECOPY, &oldProtect))
        {
            // remove short jump
            *((WORD*) procBase) = LONG_NOP;
            // remove long jump
            memset(patchBase, NOP, LONG_JMP_SIZE);

            result = TRUE;

            // restore protection
            VirtualProtect(patchBase, PATCH_SIZE, oldProtect, &oldProtect);
        }
    }

    return result;
}
