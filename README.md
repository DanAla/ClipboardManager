# Clipboard Manager

A lightweight, system tray clipboard manager built with wxWidgets for Windows.

## Features

- **System Tray Integration**: Runs in the background, accessible via system tray
- **Automatic Monitoring**: Continuously monitors clipboard for new content
- **Persistent Storage**: Saves clipboard history to file (`clipboard_history.txt`)
- **Multiple Data Types**: Detects text, images, and files
- **Easy Access**: Double-click system tray icon or right-click menu to show/hide
- **History Management**: View, copy, and clear clipboard history
- **Memory Efficient**: Limits history to 1000 entries

## Building

### Prerequisites

1. **wxWidgets**: Install wxWidgets development libraries
   - Download from: https://www.wxwidgets.org/downloads/
   - Or install via package manager (vcpkg, msys2, etc.)

2. **MinGW-w64**: GCC compiler for Windows
   - Available in MSYS2, TDM-GCC, or standalone

### Build Instructions

1. **Use the provided build script** (recommended):
   ```bash
   cd build-mingw
   build.bat
   ```

2. **Manual build with g++**:
   ```bash
   g++ -std=c++17 $(wx-config --cxxflags) -O2 -mwindows -o ClipboardManager.exe ClipboardManager.cpp $(wx-config --libs)
   ```

3. **Using CMake** (alternative):
   ```bash
   mkdir build && cd build
   cmake .. -G "MinGW Makefiles"
   mingw32-make
   ```

## Usage

### Running the Application

1. Launch `ClipboardManager.exe`
2. The application will start minimized to the system tray
3. Look for the clipboard icon in your system tray

### Basic Operations

- **Show Window**: Double-click system tray icon or right-click → "Show"
- **Copy from History**: Double-click any entry in the list or select and click "Copy Selected"
- **Clear History**: Click "Clear All" button (with confirmation)
- **Exit**: Right-click system tray icon → "Exit"

### System Tray Menu

- **Show Clipboard Manager**: Opens the main window
- **Exit**: Closes the application completely

### Features

- **Automatic Detection**: All clipboard changes are automatically captured
- **Data Types**: 
  - Text: Full content preserved
  - Images: Detected and labeled as "Image"
  - Files: File paths and metadata
- **Persistent Storage**: History survives application restarts
- **Recent First**: Most recent clips appear at the top

## File Structure

```
ClipboardManager/
├── ClipboardManager.h      # Header file
├── ClipboardManager.cpp    # Main implementation
├── CMakeLists.txt          # CMake build configuration
├── build-mingw/
│   └── build.bat          # MinGW build script
└── README.md              # This file
```

## Storage

- Clipboard history is stored in `clipboard_history.txt` in the executable directory
- Format: `timestamp|type|content`
- Newlines are escaped for proper storage/retrieval

## Limitations

- Currently Windows-only (wxWidgets is cross-platform, but system tray behavior is Windows-specific)
- Text-focused (images and files are detected but not stored/retrieved)
- Polling-based monitoring (500ms intervals)

## Customization

The application can be easily extended:

- **Monitoring Frequency**: Change timer interval in `ClipboardFrame` constructor
- **History Limit**: Modify the 1000 entry limit in `AddClipboardEntry()`
- **Data Types**: Add support for more clipboard formats in `DetermineDataType()`
- **Storage Format**: Modify `SaveToFile()` and `LoadFromFile()` for different storage backends

## Troubleshooting

### Build Issues

1. **wx-config not found**: Ensure wxWidgets is properly installed and `wx-config` is in PATH
2. **Compilation errors**: Check that you have C++17 compatible compiler
3. **Link errors**: Verify wxWidgets libraries are correctly installed

### Runtime Issues

1. **System tray not visible**: Check Windows system tray settings
2. **No clipboard detection**: Ensure application has appropriate permissions
3. **History not saving**: Check write permissions in application directory

## Future Enhancements

- Image thumbnail previews
- File content previews  
- Search functionality
- Hotkey support
- Multiple clipboard "slots"
- Cloud synchronization
- Better image/file handling
