@echo off
set _os_bitness=64
if %PROCESSOR_ARCHITECTURE% == x86 (
	if not defined PROCESSOR_ARCHITEW6432 set _os_bitness=32
)
echo Operating System is %_os_bitness% bit
if _os_bitness == 32 (
	set key="HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows"
) else (
	set key="HKLM\Software\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows"
)

reg add %key% /v AppInit_DLLs /d %~dp0%~1\hook.dll /f
reg add %key% /v LoadAppInit_DLLs /t REG_DWORD /d 1
reg add %key% /v RequireSignedAppInit_DLLs /t REG_DWORD /d 0
pause
