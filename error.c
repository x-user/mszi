#include "error.h"

void printLastError() {
	LPSTR lpMsgBuf;
	// get last error code
	DWORD dwLastError = GetLastError();

	// get message from error code
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				   FORMAT_MESSAGE_FROM_SYSTEM |
				   FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL, dwLastError,
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   (LPSTR) &lpMsgBuf, 0, NULL);

	// print error message
	printf("Error %d: %s\n", dwLastError, lpMsgBuf);
}