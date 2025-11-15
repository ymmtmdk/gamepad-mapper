@echo off

REM Call vcvars64.bat to set up the Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Define the CMAKE variable
set "CMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

REM Run CMake to configure the project
"%CMAKE%" -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="c:\Users\tuftu\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

REM Run CMake to build the project
"%CMAKE%" --build build --config Release --parallel
