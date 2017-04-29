#include "injector.h"

DWORD codeBase;
InjectStruct inject;
PROCESS_INFORMATION lpProcessInformation;

int __cdecl main() {
	BOOL result = FALSE;

	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc < 3) {
		wprintf(L"Usage: %s filename hook\n", argv[0]);
		printf("where:\n");
		printf("    filename    path to target executable\n");
		printf("    hook        path to library for injection\n");
		return 1;
	}

	STARTUPINFOW lpStartupInfo = {0};
	
	wprintf(L"Launching %s in paused state.. ", argv[1]);
	if (!CreateProcessW(argv[1],
						NULL,
						NULL,
						NULL,
						FALSE,
						CREATE_SUSPENDED,
						NULL,
						NULL,
						&lpStartupInfo,
						&lpProcessInformation))
	{
		printLastError();
	}
	else {
		wprintf(L"success\nInjecting %s in %s.. ", argv[2], argv[1]);
		if (!injectFunct(argv[2])) {
			printLastError();
		}
		else {
			wprintf(L"success\nResuming main thread of %s.. ", argv[1]);
			if (-1 != ResumeThread(lpProcessInformation.hThread)) {
				result = TRUE;
				printf("success\n");
			}
			else {
				printLastError();
				wprintf(L"Attempting to kill %s.. ", argv[1]);
				if (!TerminateProcess(lpProcessInformation.hProcess, 1)) {
					printLastError();
				}
				else {
					printf("success\n");
				}
			}
		}

		// close handles
		CloseHandle(lpProcessInformation.hThread);
		CloseHandle(lpProcessInformation.hProcess);
	}
	
	return !result;
}

BOOL injectFunct(LPWSTR lpHookDll) {
	BOOL result = FALSE;
	WCHAR lpFullPath[MAX_PATH];

	// get full path to hook.dll
	if (GetFullPathNameW(lpHookDll, MAX_PATH, lpFullPath, NULL)) {
		// get handle of kernel32.dll
		HANDLE hKernl = GetModuleHandleA("kernel32.dll");

		if (hKernl) {
			// allocate memory in target process
			PVOID mem = VirtualAllocEx(lpProcessInformation.hProcess,
									   NULL,
									   sizeof(inject),
									   MEM_TOP_DOWN |
									   MEM_COMMIT,
									   PAGE_EXECUTE_READWRITE);

			// memory successfully allocated
			if (mem) {
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
				if (WriteProcessMemory(lpProcessInformation.hProcess,
									   mem,
									   &inject,
									   (SIZE_T) sizeof(inject),
									   NULL))
				{
					// start injected code in target process
					HANDLE hThread = CreateRemoteThread(lpProcessInformation.hProcess,
														NULL,
														0,
														(LPTHREAD_START_ROUTINE) mem,
														NULL,
														0,
														NULL);

					if (hThread) {
						// wait until remote thread finish it's work
						WaitForSingleObject(hThread, INFINITE);
						
						result = TRUE;
						// close remote thread handle
						CloseHandle(hThread);
					}
				}

				// free allocated memory
				VirtualFreeEx(lpProcessInformation.hProcess, mem, 0, MEM_RELEASE);
			}

			// close kernel32.dll module handle
			CloseHandle(hKernl);
		}
	}

	return result;
}

DWORD rebasePtr(PVOID ptr) {
	return codeBase + (DWORD) ptr - (DWORD) &inject;
}