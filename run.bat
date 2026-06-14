@echo off
setlocal
for /F %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

cls
echo made in the hellenic republic :)

REM -----------------------------
REM Check if Docker is running
REM -----------------------------
docker info >nul 2>&1
if errorlevel 1 (
    echo Docker is not running. Starting Docker Desktop...
    start "" "C:\Program Files\Docker\Docker\Docker Desktop.exe"
    echo Waiting for Docker to start...
    :waitloop
    timeout /t 3 >nul
    docker info >nul 2>&1
    if errorlevel 1 goto waitloop
    echo Docker started.
)

REM -----------------------------
REM Build the kernel in Docker
REM -----------------------------
echo Building ModuOS...

REM Capture start time
for /f "tokens=1-3 delims=:." %%a in ("%time%") do (
    set /a "start_h=%%a", "start_m=1%%b-100", "start_s=1%%c-100"
)

docker run --rm -it --privileged -v /dev:/dev -v "%cd%":/root/env modu-os /bin/bash -c "cd /root/env && make -j12 clean && make -j12 all"

REM Capture end time
for /f "tokens=1-3 delims=:." %%a in ("%time%") do (
    set /a "end_h=%%a", "end_m=1%%b-100", "end_s=1%%c-100"
)

REM Calculate duration
set /a "duration=(end_h*3600 + end_m*60 + end_s) - (start_h*3600 + start_m*60 + start_s)"
if %duration% lss 0 set /a duration+=86400

REM -----------------------------
REM Results
REM -----------------------------
if %errorlevel% neq 0 (
    echo.
    echo %ESC%[31mBUILD FAILED [%duration% seconds]%ESC%[0m
    exit /b 1
)

echo.
echo %ESC%[32mBuild Complete [%duration% seconds]%ESC%[0m
echo %ESC%[38;2;255;165;0mCompiled .SQR binaries are located in the dist/ directory.%ESC%[0m