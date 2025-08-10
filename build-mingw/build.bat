@echo off
echo Building Clipboard Manager with MinGW...

REM Check if ClipboardManager is running and kill it
echo Checking for running ClipboardManager processes...
tasklist /FI "IMAGENAME eq ClipboardManager.exe" 2>NUL | find /I /N "ClipboardManager.exe" >NUL
if "%ERRORLEVEL%"=="0" (
    echo Stopping ClipboardManager...
    taskkill /F /IM ClipboardManager.exe >NUL 2>&1
    timeout /T 2 /NOBREAK >NUL
    echo ClipboardManager stopped.
) else (
    echo ClipboardManager is not running.
)
echo.

REM Set build directory
set BUILD_DIR=%~dp0
set PROJECT_DIR=%BUILD_DIR%..

REM Set wxWidgets paths
set WX_DIR=C:\wxWidgets-3.3.1
set WX_LIB_DIR=%WX_DIR%\build-mingw\lib\gcc_x64_lib
set WX_SETUP_DIR=%WX_LIB_DIR%\mswu

REM Check if wxWidgets is available
echo Checking for wxWidgets...
if not exist "%WX_DIR%\include\wx\wx.h" (
    echo Error: wxWidgets headers not found
    echo Expected location: %WX_DIR%\include\wx\wx.h
    exit /b 1
)

REM Set wxWidgets compile and link flags
set WX_CXXFLAGS=-I"%WX_DIR%\include" -I"%WX_SETUP_DIR%" -D__WXMSW__
set WX_LIBS=-L"%WX_LIB_DIR%" -lwxmsw33u_core -lwxbase33u -lwxmsw33u_adv -lwxpng -lwxzlib -lwxjpeg -lwxexpat -lwxlexilla
REM Add Windows system libraries
set SYS_LIBS=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -lcomctl32 -lrpcrt4 -lwinmm -luxtheme -lgdiplus -lshlwapi -lmsimg32 -loleacc -lversion

echo wxWidgets found and configured

REM Create output directory
if not exist "%BUILD_DIR%output" mkdir "%BUILD_DIR%output"

REM Compile the resource file
echo Compiling resources...
windres "%PROJECT_DIR%\ClipboardManager.rc" -O coff -o "%BUILD_DIR%temp_resources.o"

if %errorlevel% neq 0 (
    echo Resource compilation failed!
    exit /b 1
)

REM Compile the application
echo Compiling ClipboardManager...
g++ -std=c++17 ^
    %WX_CXXFLAGS% ^
    -I"%PROJECT_DIR%" ^
    -O2 ^
    -mwindows ^
    -static-libgcc ^
    -static-libstdc++ ^
    -static ^
    -o "%BUILD_DIR%output\ClipboardManager.exe" ^
    "%PROJECT_DIR%\ClipboardManager.cpp" ^
    "%BUILD_DIR%temp_resources.o" ^
    %WX_LIBS% %SYS_LIBS%

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

REM Clean up temporary files
if exist "%BUILD_DIR%temp_resources.o" del "%BUILD_DIR%temp_resources.o"

echo Build successful!
echo Executable created: %BUILD_DIR%output\ClipboardManager.exe
echo.
echo Starting ClipboardManager...
start "" "%BUILD_DIR%output\ClipboardManager.exe"
timeout /T 1 /NOBREAK >NUL
echo.
echo ClipboardManager is now running minimized to the system tray.
echo Left-click the tray icon to show the window.
echo Right-click the tray icon and select "Exit" to quit the application.
