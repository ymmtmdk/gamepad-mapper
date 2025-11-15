@echo off

REM Call vcvars64.bat to set up the Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Run CMake to configure the project
cmake -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

REM Run CMake to build the project
cmake --build build --config Release --parallel
