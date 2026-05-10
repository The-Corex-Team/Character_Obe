@echo off
setlocal
cd /d "%~dp0"

:: ── Kill running instance so the linker can overwrite the exe ────────────────
taskkill /f /im CharacterObe.exe >nul 2>&1

:: ── Qt6 path ──────────────────────────────────────────────────────────────────
:: Override by setting QT_DIR before calling this script, e.g.:
::   set QT_DIR=C:\Qt\6.9.0\msvc2022_64  && build.bat
if not defined QT_DIR set "QT_DIR=C:\Qt\6.8.0\msvc2022_64"
set "CMAKE_PREFIX_ARG=-DCMAKE_PREFIX_PATH=%QT_DIR%"

:: ── Configure & build ─────────────────────────────────────────────────────────
if not exist build mkdir build
pushd build

cmake .. -G "Visual Studio 17 2022" -A x64 %CMAKE_PREFIX_ARG%
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] CMake configure failed.
    popd & exit /b 1
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed.
    popd & exit /b 1
)

popd
echo.
echo Build complete.  Output: build\Release\CharacterObe.exe
