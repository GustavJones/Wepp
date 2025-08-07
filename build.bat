@echo off

:: Set current directory to script directory
cd /d "%~dp0"

:: Configure cmake project
cmake.exe -S . -B build -DCMAKE_BUILD_TYPE=Debug

:: Build cmake project
cmake --build build --config Debug -j
