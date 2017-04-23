#include "injector.h"

DWORD codeBase;
InjectStruct inject;

int main(int argc, char* argv[]) {

	if (2 != argc) {
		printf("Usage: %s filename\nWhere\n", argv[0]);
		printf("\tfilename\tpath to target programm executable.\n");
		return 2;
	}

	STARTUPINFO startupinfo;
	PROCESS_INFORMATION procinfo;

	// init structure with zeros
	memset(&startupinfo, 0, sizeof(startupinfo));

	printf("Starting %s in paused state.. ", argv[1]);
	if (!CreateProcessA(
			argv[1],
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&startupinfo,
			&procinfo))
	{
		printf("fail.");
		return 1;
	} else {
		printf("success.\nInjecting hook.dll in %s process.. ", argv[1]);
		if (!injectFunct(procinfo.dwProcessId))
			printf("fail.\n");
		else
			printf("success.\n");

		printf("Resuming %s process main thread.. ", argv[1]);
		if (-1 == ResumeThread(procinfo.hThread))
			printf("fail.\n");
		else
			printf("success.\n");

		CloseHandle(procinfo.hThread);
		CloseHandle(procinfo.hProcess);
		return 0;
}	}

DWORD rebasePtr(PVOID ptr) {

	return codeBase + (DWORD) ptr - (DWORD) &inject;
}

BOOL injectFunct(DWORD dwProcId) {

	BOOL result = FALSE;
	char *path = (char*) malloc(MAX_PATH);

	if (GetFullPathName("hook.dll", MAX_PATH, path, NULL)) {

		// open target process with write memory access
		HANDLE hProc = OpenProcess(
				PROCESS_CREATE_THREAD |
				PROCESS_VM_OPERATION |
				PROCESS_VM_READ | PROCESS_VM_WRITE |
				PROCESS_QUERY_INFORMATION,
				FALSE,
				dwProcId);

		if (hProc) {
			// get handle of kernel32.dll
			HANDLE hKernl = GetModuleHandle("kernel32.dll");

			if (hKernl) {
				// allocate memory in target process
				PVOID mem = VirtualAllocEx(
						hProc,
						NULL,
						sizeof(inject),
						MEM_TOP_DOWN |
						MEM_COMMIT,
						PAGE_EXECUTE_READWRITE);

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
					inject.cmd2ar = rebasePtr(&(inject.pLoadLibraryA));       //
					// ExitThread(0); --------------------------------------- //
					inject.cmd3 = PUSH;           // 0x68                     //
					inject.cmd3ar = 0;            // 0x00                     //
					inject.cmd4 = CALL_DWORD_PTR; // 0x15FF                   //
					inject.cmd4ar = rebasePtr(&(inject.pExitThread));         //
					// ---------------------------------------[ data block ]- //
					// function pointers ------------------------------------ //
					inject.pExitThread =                                      //
						GetProcAddress(hKernl, "ExitThread");                 //
					inject.pLoadLibraryA =                                    //
						GetProcAddress(hKernl, "LoadLibraryA");               //
					// strings ---------------------------------------------- //
					strncpy(inject.LibraryPath, path, MAX_PATH);              //
					// ====================================================== //

					// perform injection
					if (WriteProcessMemory(
							hProc,
							mem,
							&inject,
							(SIZE_T) sizeof(inject),
							NULL))
					{
						// start injected code in target process
						HANDLE hThread = CreateRemoteThread(
								hProc,
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
							CloseHandle(hThread);
					}	}

					VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
				}

				CloseHandle(hKernl);
			}

			CloseHandle(hProc);
	}	}

	free(path);
	return result;
}