#pragma once
#include <windows.h>

// Application Configuration
namespace AppConstants {
    // Window settings
    constexpr int WINDOW_WIDTH = 800;
    constexpr int WINDOW_HEIGHT = 600;
    constexpr DWORD FRAME_SLEEP_MS = 20;
    
    // Input configuration
    constexpr size_t MAX_BUTTONS = 128;
    constexpr size_t AXIS_DIRECTIONS = 4;
    constexpr LONG AXIS_THRESHOLD_DEFAULT = 400;
    
    // DirectInput settings
    constexpr LONG AXIS_RANGE_MIN = -1000;
    constexpr LONG AXIS_RANGE_MAX = 1000;
    
    // Logging settings
    constexpr size_t LOG_BUFFER_SIZE = 1024;
    constexpr size_t FRAME_LOG_MAX_LINES = 50;
}

// Axis direction enumeration
enum AxisDirection { 
    AX_LEFT = 0, 
    AX_RIGHT = 1, 
    AX_UP = 2, 
    AX_DOWN = 3 
};