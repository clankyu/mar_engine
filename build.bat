@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

set PROJECT_ROOT=%~dp0
set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%

cmake -B build -S . -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

cmake --build build --config Release

if %ERRORLEVEL% EQU 0 (
    echo [BUILD SUCCESS]
    start raddbg --ipc select_target %PROJECT_ROOT%\build\engine.exe
)
