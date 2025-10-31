#pragma once
#include <windows.h>
#include <dinput.h>
#include <string>
#include <memory>
#include "Constants.h"
#include "ILogger.h"

// Forward declarations
class WindowManager;
class MultipleGamepadManager;
class IDisplayBuffer;

/**
 * @brief Main application class with multiple gamepad support
 * 
 * This is the updated Application class that supports multiple gamepad devices
 * simultaneously, each with their own configuration files and input processing.
 */
class Application {
public:
    // Constructor/Destructor (RAII)
    explicit Application(HINSTANCE hInstance);
    ~Application();
    
    // Non-copyable, but movable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = default;
    Application& operator=(Application&&) = default;
    
    // Main application lifecycle
    bool Initialize();
    int Run();
    void Shutdown();
    
    // Application state
    bool IsRunning() const { return m_running; }
    void RequestExit() { m_running = false; }

private:
    // Initialization methods
    bool InitializeLogger();
    bool InitializeWindow();
    bool InitializeGamepadManager();
    
    // Main loop methods
    void ProcessMessages();
    void UpdateFrame();
    void BuildFrameContent();
    void ProcessGamepadInput();
    void UpdateDisplay();
    void CheckExitConditions();
    
    // Helper methods
    std::string GenerateLogPath() const;
    void CleanupResources();
    void LogGamepadStatus();
    
    // Application components (dependency injection pattern)
    HINSTANCE m_hInstance;
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<MultipleGamepadManager> m_gamepadManager;
    std::unique_ptr<IDisplayBuffer> m_displayBuffer;
    
    // Application state
    bool m_running;
    bool m_initialized;
    
    // Runtime data
    MSG m_msg;
    
    // Configuration
    static constexpr int WINDOW_WIDTH = AppConstants::WINDOW_WIDTH;
    static constexpr int WINDOW_HEIGHT = AppConstants::WINDOW_HEIGHT;
    static constexpr DWORD FRAME_SLEEP_MS = AppConstants::FRAME_SLEEP_MS;
};