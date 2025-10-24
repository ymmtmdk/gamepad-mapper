#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Application.h"

/**
 * @brief Main entry point for the multiple gamepad mapper application
 * 
 * This version supports multiple gamepads simultaneously, each with
 * their own configuration files and input processing.
 */
int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    Application app(hInst);

    if (!app.Initialize()) {
        return 1;
    }
    
    // Run the application
    return app.Run();
}