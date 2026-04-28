@echo off
setlocal
set "DEPS_CONFIG=%~1"
if "%DEPS_CONFIG%"=="" set "DEPS_CONFIG=debug"
set "DEPS_EXTRA="
if /I "%DEPS_CONFIG%"=="debug" set "DEPS_EXTRA=skip-release"
if /I not "%DEPS_CONFIG%"=="debug" if /I not "%DEPS_CONFIG%"=="both" (
  echo [arcanegram] usage: prepare-deps.bat [debug^|both]
  exit /b 2
)
call "%~dp0setup-vcvars.bat"
if errorlevel 1 exit /b 1
set "QT=6.8"
:: prepare.py invokes "command.bat" without `.\` prefix; needs cwd in exec search.
set "NoDefaultCurrentDirectoryInExePath="
echo [arcanegram] platform=%Platform% config=%DEPS_CONFIG%
cd /d "%~dp0worktree"
call Telegram\build\prepare\win.bat qt6 %DEPS_EXTRA% silent
exit /b %errorlevel%
