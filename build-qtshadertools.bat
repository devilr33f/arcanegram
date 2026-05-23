@echo off
setlocal
:: build only the qtshadertools module against the already-installed static Qt,
:: to produce qsb.exe (Qt Shader Baker) required by upstream Telegram/shaders.
call "%~dp0setup-vcvars.bat"
if errorlevel 1 exit /b 1

set "LIBS_DIR=%~dp0Libraries\win64"
set "QT_PREFIX=%LIBS_DIR%\Qt-6.11.0"
set "MODULE_SRC=%LIBS_DIR%\qt_6.11.0\qtshadertools"
set "BUILD_DIR=%LIBS_DIR%\qt_6.11.0\qtshadertools-build"

if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

set "ZLIB_DIR=%LIBS_DIR%\zlib"
set "MOZJPEG_DIR=%LIBS_DIR%\mozjpeg"
set "WEBP_DIR=%LIBS_DIR%\libwebp"
set "LCMS2_DIR=%LIBS_DIR%\liblcms2"
:: qsb requires Qt6Gui; Gui's config resolves only when its system deps (the same
:: ones prepare.py fed the full Qt build) are locatable. mirror those hints here.
echo [arcanegram] === configure qtshadertools module ===
call "%QT_PREFIX%\bin\qt-configure-module.bat" "%MODULE_SRC%" -- ^
    -D ZLIB_FOUND=1 ^
    -D ZLIB_INCLUDE_DIR="%ZLIB_DIR%" ^
    -D ZLIB_LIBRARY_DEBUG="%ZLIB_DIR%\Debug\zlibstaticd.lib" ^
    -D ZLIB_LIBRARY_RELEASE="%ZLIB_DIR%\Release\zlibstatic.lib" ^
    -D JPEG_FOUND=1 ^
    -D JPEG_INCLUDE_DIR="%MOZJPEG_DIR%" ^
    -D JPEG_LIBRARY_DEBUG="%MOZJPEG_DIR%\Debug\jpeg-static.lib" ^
    -D JPEG_LIBRARY_RELEASE="%MOZJPEG_DIR%\Release\jpeg-static.lib" ^
    -D WebP_INCLUDE_DIR="%WEBP_DIR%\src" ^
    -D WebP_demux_INCLUDE_DIR="%WEBP_DIR%\src" ^
    -D WebP_mux_INCLUDE_DIR="%WEBP_DIR%\src" ^
    -D WebP_LIBRARY="%WEBP_DIR%\out\release-static\x64\lib\webp.lib" ^
    -D WebP_demux_LIBRARY="%WEBP_DIR%\out\release-static\x64\lib\webpdemux.lib" ^
    -D WebP_mux_LIBRARY="%WEBP_DIR%\out\release-static\x64\lib\webpmux.lib" ^
    -D LCMS2_FOUND=1 ^
    -D LCMS2_INCLUDE_DIR="%LCMS2_DIR%\include" ^
    -D LCMS2_LIBRARIES="%LCMS2_DIR%\out\Release\src\liblcms2.a"
if errorlevel 1 exit /b 1

:: generator is ninja multi-config with configs RelWithDebInfo;Debug.
:: qsb (host tool) builds under the default RelWithDebInfo config.
echo [arcanegram] === build/install RelWithDebInfo ===
cmake --build . --config RelWithDebInfo
if errorlevel 1 exit /b 1
cmake --install . --config RelWithDebInfo
if errorlevel 1 exit /b 1

echo [arcanegram] === build/install Debug ===
cmake --build . --config Debug
if errorlevel 1 exit /b 1
cmake --install . --config Debug
if errorlevel 1 exit /b 1

echo [arcanegram] === done; qsb should be in %QT_PREFIX%\libexec ===
exit /b 0
