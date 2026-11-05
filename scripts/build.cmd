@echo off
rem Release build of the plugin DLL with MSVC + Ninja.
rem Override paths with environment variables if your setup differs:
rem   VCVARS   - path to vcvars64.bat
rem   QT_DIR   - Qt 6.8.x msvc2022_64 prefix
rem   TOOLS    - directory with cmake.exe and ninja.exe

setlocal
if "%VCVARS%"=="" set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if "%QT_DIR%"=="" set "QT_DIR=%USERPROFILE%\Qt\6.8.3\msvc2022_64"
if "%TOOLS%"=="" set "TOOLS=%USERPROFILE%\.openrgb-toolchain\venv\Scripts"

call "%VCVARS%" >nul || exit /b 1
set "PATH=%TOOLS%;%PATH%"

cd /d "%~dp0.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%" || exit /b 1
cmake --build build || exit /b 1
echo.
echo Built: %CD%\build\OpenRGBViaRGBMatrixPlugin.dll
