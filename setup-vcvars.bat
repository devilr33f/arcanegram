@echo off
:: locate and source vcvars64 for x64. idempotent — skips if already in vc env.
:: no setlocal: env vars (vcinstalldir, path, include, lib, ...) must propagate to caller.
:: parenthesized `if` blocks are avoided because expanded paths contain `(x86)` which would close blocks early.
if defined VCINSTALLDIR exit /b 0
set "_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%_VSWHERE%" goto :no_vswhere
set "_VSPATH="
for /f "usebackq tokens=*" %%i in (`"%_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "_VSPATH=%%i"
if not defined _VSPATH goto :no_vsinstall
set "_VCVARS_ARGS="
if defined MSVC_VER set "_VCVARS_ARGS=-vcvars_ver=%MSVC_VER%"
call "%_VSPATH%\VC\Auxiliary\Build\vcvars64.bat" %_VCVARS_ARGS%
exit /b %errorlevel%

:no_vswhere
echo [arcanegram] vswhere.exe not found at %_VSWHERE%
exit /b 1

:no_vsinstall
echo [arcanegram] vswhere found no vs install with vc tools
exit /b 1
