#include "injector.h"

DWORD codeBase;
InjectStruct inject;
PROCESS_INFORMATION lpProcInfo;

int __cdecl main()
{
    int argc;
    BOOL result = FALSE;
    // create startup info structure and fill it with zeros
    // otherwise CreateProcessW will not work.
    STARTUPINFOW lpStartupInfo = {0};

    // check command line arguments
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < 3)
    {
        fwprintf(stdout, L"Usage: %s filename hook\n", argv[0]);
        fprintf(stdout, "where:\n");
        fprintf(stdout, "    filename    path to target executable\n");
        fprintf(stdout, "    hook        path to library for injection\n");
        return 1;
    }

    // start target application in suspended state
    fwprintf(stdout, L"Launching %s in paused state.. ", argv[1]);
    if (!CreateProcessW(argv[1],
                        NULL,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_SUSPENDED,
                        NULL,
                        NULL,
                        &lpStartupInfo,
                        &lpProcInfo))
    {
            printLastError();
    }
    else
    {
        // perform code injection
        fwprintf(stdout, L"success\nInjecting %s in %s.. ", argv[2], argv[1]);
        if (!injectFunct(argv[2]))
        {
            printLastError();
        }
        else
        {
            // resume target application
            fwprintf(stdout, L"success\nResuming main thread of %s.. ", argv[1]);
            if (-1 != ResumeThread(lpProcInfo.hThread))
            {
                result = TRUE;
                fprintf(stdout, "success\n");

                // wait
                WaitForSingleObject(lpProcInfo.hThread, INFINITE);
            }
            else
            {
                // resuming failed attempting to kill target application
                printLastError();
                fwprintf(stdout, L"Attempting to kill %s.. ", argv[1]);
                if (!TerminateProcess(lpProcInfo.hProcess, 1))
                {
                    printLastError();
                }
                else
                {
                    fprintf(stdout, "success\n");
                }
            }
        }

        // close handles
        CloseHandle(lpProcInfo.hThread);
        CloseHandle(lpProcInfo.hProcess);
    }

    return !result;
}

BOOL injectFunct(LPWSTR lpHookDll)
{
    BOOL result = FALSE;
    WCHAR lpFullPath[MAX_PATH];

    // get full path to hook library
    if (GetFullPathNameW(lpHookDll, MAX_PATH, lpFullPath, NULL))
    {
        // get handle of kernel32.dll
        HANDLE hKernl = GetModuleHandleA("kernel32.dll");

        if (hKernl)
        {
            // allocate memory in target process
            PVOID mem = VirtualAllocEx(lpProcInfo.hProcess,
                                       NULL,
                                       sizeof(inject),
                                       MEM_TOP_DOWN |
                                       MEM_COMMIT,
                                       PAGE_EXECUTE_READWRITE);

            // memory successfully allocated
            if (mem)
            {
                // store allocated memory block address
                codeBase = (DWORD) mem;

                // ====================================================== //
                // fill structure                                         //
                // ====================================================== //
                // ---------------------------------------[ code block ]- //
                inject.cmd0 = NOP;            // 0x90                     //
                // LoadLibraryA(LibraryPath); --------------------------- //
                inject.cmd1 = PUSH;           // 0x68                     //
                inject.cmd1ar = rebasePtr(&(inject.LibraryPath));         //
                inject.cmd2 = CALL_DWORD_PTR; // 0x15FF                   //
                inject.cmd2ar = rebasePtr(&(inject.pLoadLibraryW));       //
                // ExitThread(0); --------------------------------------- //
                inject.cmd3 = PUSH;           // 0x68                     //
                inject.cmd3ar = 0;            // 0x00                     //
                inject.cmd4 = CALL_DWORD_PTR; // 0x15FF                   //
                inject.cmd4ar = rebasePtr(&(inject.pExitThread));         //
                // ---------------------------------------[ data block ]- //
                // function pointers ------------------------------------ //
                inject.pExitThread =                                      //
                    GetProcAddress(hKernl, "ExitThread");                 //
                inject.pLoadLibraryW =                                    //
                    GetProcAddress(hKernl, "LoadLibraryW");               //
                // strings ---------------------------------------------- //
                wcsncpy(inject.LibraryPath, lpFullPath, MAX_PATH);        //
                // ====================================================== //

                // perform injection
                if (WriteProcessMemory(lpProcInfo.hProcess,
                                       mem,
                                       &inject,
                                       (SIZE_T) sizeof(inject),
                                       NULL))
                {
                    // start injected code in target process
                    HANDLE hThread = CreateRemoteThread(lpProcInfo.hProcess,
                                                        NULL,
                                                        0,
                                                        (LPTHREAD_START_ROUTINE) mem,
                                                        NULL,
                                                        0,
                                                        NULL);

                    if (hThread)
                    {
                        // wait until remote thread finish it's work
                        WaitForSingleObject(hThread, INFINITE);

                        result = TRUE;
                        // close remote thread handle
                        CloseHandle(hThread);
                    }
                }

                // free allocated memory
                VirtualFreeEx(lpProcInfo.hProcess, mem, 0, MEM_RELEASE);
            }

            // close kernel32.dll module handle
            CloseHandle(hKernl);
        }
    }

    return result;
}

DWORD rebasePtr(PVOID ptr)
{
    return codeBase + (DWORD) ptr - (DWORD) &inject;
}
