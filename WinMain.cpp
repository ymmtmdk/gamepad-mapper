#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Application.h"
#include <iostream>
#include <fstream>

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    Application app(hInst);

    if (!app.Initialize()) {
        return 1;
    }
    
    // Run the application
    return app.Run();
}
