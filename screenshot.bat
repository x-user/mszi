@echo off
del screenshot.bmp
screenshot.exe
echo.
injector.exe screenshot.exe hook.dll
injector.exe screenshot.exe hook_new.dll
pause