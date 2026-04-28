@echo off
setlocal
set "BUILD_CONFIG=%~1"
if "%BUILD_CONFIG%"=="" set "BUILD_CONFIG=debug"
if /I "%BUILD_CONFIG%"=="configure" goto :ok
if /I "%BUILD_CONFIG%"=="debug" goto :ok
if /I "%BUILD_CONFIG%"=="release" goto :ok
if /I "%BUILD_CONFIG%"=="both" goto :ok
echo [arcanegram] usage: build-telegram.bat [debug^|release^|both^|configure]
exit /b 2

:ok
call "%~dp0setup-vcvars.bat"
if errorlevel 1 exit /b 1
set "QT=6.8"
set "NoDefaultCurrentDirectoryInExePath="
if not defined TDESKTOP_API_ID set "TDESKTOP_API_ID=2040"
if not defined TDESKTOP_API_HASH set "TDESKTOP_API_HASH=b18441a1ff607e10a989891a5462e627"
if "%TDESKTOP_API_TEST%"=="1" (set "EXTRA_FLAGS=-DTDESKTOP_API_TEST=ON") else (set "EXTRA_FLAGS=-DTDESKTOP_API_TEST=OFF")
echo [arcanegram] platform=%Platform% config=%BUILD_CONFIG% api_id=%TDESKTOP_API_ID%
cd /d "%~dp0worktree"

echo [arcanegram] === configure ===
call Telegram\configure.bat qt6 x64 -DTDESKTOP_API_ID=%TDESKTOP_API_ID% -DTDESKTOP_API_HASH=%TDESKTOP_API_HASH% -DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT= %EXTRA_FLAGS%
if errorlevel 1 exit /b 1

if /I "%BUILD_CONFIG%"=="configure" exit /b 0
if /I "%BUILD_CONFIG%"=="release" goto :do_release

echo [arcanegram] === build debug ===
cmake --build out --config Debug --target Telegram
if errorlevel 1 exit /b %errorlevel%
if /I "%BUILD_CONFIG%"=="debug" exit /b 0

:do_release
echo [arcanegram] === build release ===
cmake --build out --config Release --target Telegram
exit /b %errorlevel%
