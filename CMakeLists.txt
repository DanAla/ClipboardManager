cmake_minimum_required(VERSION 3.16)

project(ClipboardManager)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base adv)

if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
    
    # Add executable
    add_executable(ClipboardManager
        ClipboardManager.cpp
        ClipboardManager.h
    )
    
    # Link wxWidgets libraries
    target_link_libraries(ClipboardManager ${wxWidgets_LIBRARIES})
    
    # Set target properties for Windows
    if(WIN32)
        set_target_properties(ClipboardManager PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
    
else()
    message(FATAL_ERROR "wxWidgets not found!")
endif()

# Additional compiler flags for Windows
if(MSVC)
    target_compile_definitions(ClipboardManager PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WXUSINGDLL
    )
endif()
