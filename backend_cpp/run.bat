@echo off
REM Quick start script for C++ backend (Windows)

cd /d "%~dp0"

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

cd build

REM Configure and build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release

REM Run the backend
echo Starting backend server on port 8080...
Release\transport_backend.exe 8080

pause

