#pragma once
#include <windows.h>
#include <dinput.h>
#include <string>
#include <memory>
#include "Constants.h"
#include "ILogger.h"

#include "JsonConfigManager.h"

// Forward declarations
class WindowManager;
class InputProcessor;
class DirectInputManager;

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
    bool InitializeConfiguration();
    bool InitializeWindow();
    bool InitializeDirectInput();
    bool InitializeInputProcessor();
    
    // Main loop methods
    void ProcessMessages();
    void UpdateFrame();
    void ProcessGamepadInput();
    void UpdateDisplay();
    void CheckExitConditions();
    
    // Helper methods
    std::string GenerateLogPath() const;
    void CleanupResources();
    
    // Application components (dependency injection pattern)
    HINSTANCE m_hInstance;
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<JsonConfigManager> m_configManager;
    std::unique_ptr<InputProcessor> m_inputProcessor;
    std::unique_ptr<DirectInputManager> m_directInputManager;
    
    // Application state
    bool m_running;
    bool m_initialized;
    
    // Runtime data
    MSG m_msg;
    DIJOYSTATE2 m_joystickState;
    
    // Configuration
    static constexpr int WINDOW_WIDTH = AppConstants::WINDOW_WIDTH;
    static constexpr int WINDOW_HEIGHT = AppConstants::WINDOW_HEIGHT;
    static constexpr DWORD FRAME_SLEEP_MS = AppConstants::FRAME_SLEEP_MS;
};