@echo off
echo Running ClipboardManager in debug mode...

REM Create a simple test to run the app and capture any output
cd build-mingw\output

echo Starting application...
ClipboardManager.exe 2>&1

echo.
echo Application finished. Checking for log files...
dir *.txt *.log 2>nul
