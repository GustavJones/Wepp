@echo off

:: Set current directory to script directory
cd /d "%~dp0"

:: Configure cmake project
cmake.exe -S . -B build -DCMAKE_BUILD_TYPE=Debug

:: Set amount of jobs
set /a JOBS=%NUMBER_OF_PROCESSORS% - 1

:: Build cmake project
cmake --build build --config Debug -j %JOBS% 
