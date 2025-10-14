@echo off

:: CMake Build Type
set BUILD_TYPE=Debug

:: Set current directory to script directory
cd /d "%~dp0"

:: Configure cmake project
cmake.exe -S . -B build -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

:: Set amount of jobs
set /a JOBS=%NUMBER_OF_PROCESSORS% - 1

:: Build cmake project
cmake --build build --config %BUILD_TYPE% -j %JOBS%
