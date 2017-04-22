@echo off
set _os_bitness=64
if %PROCESSOR_ARCHITECTURE% == x86 (
	if not defined PROCESSOR_ARCHITEW6432 set _os_bitness=32
)
echo Operating System is %_os_bitness% bit
if _os_bitness == 32 (
	set key=HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows
) else (
	set key=HKLM\Software\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows
)

echo.
echo This will change following registry values:

echo.
echo %key%
echo    AppInit_DLLs to ""
echo    LoadAppInit_DLLs to 0
echo    RequireSignedAppInit_DLLs to 1

echo.
set /p confirm=Are you sure (Y/[N])? 
if /i "%confirm%" == "Y" (
	echo.
	echo Unistalling..
	reg add "%key%" /v AppInit_DLLs /d "" /f
	reg add "%key%" /v LoadAppInit_DLLs /t REG_DWORD /d 0 /f
	reg add "%key%" /v RequireSignedAppInit_DLLs /t REG_DWORD /d 1 /f
)

pause