#include "Application.h"
#include "WindowManager.h"
#include "JsonConfigManager.h"
#include "InputProcessor.h"
#include "DirectInputManager.h"
#include "Logger.h"
#include <memory>
#include <stdexcept>
#include <filesystem>

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_running(false)
    , m_initialized(false)
    , m_msg{}
    , m_joystickState{}
{
}

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize()
{
    if (m_initialized) {
        return true;
    }
    
    try {
        // Initialize all components in dependency order
        if (!InitializeLogger()) {
            throw std::runtime_error("Logger initialization failed");
        }
        
        if (!InitializeConfiguration()) {
            CleanupResources();
            throw std::runtime_error("Configuration initialization failed");
        }
        
        if (!InitializeWindow()) {
            CleanupResources();
            throw std::runtime_error("Window initialization failed");
        }
        
        if (!InitializeDirectInput()) {
            CleanupResources();
            throw std::runtime_error("DirectInput initialization failed");
        }
        
        if (!InitializeInputProcessor()) {
            CleanupResources();
            throw std::runtime_error("InputProcessor initialization failed");
        }
        
        m_initialized = true;
        LOG_WRITE("Application initialization completed successfully.");
        return true;
        
    } catch (const std::exception& e) {
        LOG_WRITE("Initialization failed: %s", e.what());
        CleanupResources();
        return false;
    }
}

bool Application::InitializeLogger()
{
    std::string logPath = GenerateLogPath();
    if (!Logger::GetInstance().Init(logPath)) {
        MessageBox(nullptr, L"Failed to initialize log file!", L"Error", MB_ICONERROR);
        return false;
    }
    return true;
}

bool Application::InitializeConfiguration()
{
    m_configManager = std::make_unique<JsonConfigManager>();
    if (!m_configManager->isLoaded()) {
        MessageBox(nullptr, L"Failed to load gamepad configuration!", L"Error", MB_ICONERROR);
        return false;
    }
    
    return true;
}

bool Application::InitializeWindow()
{
    m_windowManager = std::make_unique<WindowManager>(m_hInstance, L"Gamepad Mapper", &Logger::GetInstance());
    if (!m_windowManager->Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        MessageBox(nullptr, L"Window initialization failed!", L"Error", MB_ICONERROR);
        return false;
    }
    return true;
}

bool Application::InitializeDirectInput()
{
    m_directInputManager = std::make_unique<DirectInputManager>();
    if (!m_directInputManager->Initialize(m_hInstance, m_windowManager->GetHwnd())) {
        // This will only fail in case of a fatal error, like DirectInput8Create failing.
        MessageBox(nullptr, L"A critical error occurred while initializing DirectInput.", L"Fatal Error", MB_ICONERROR);
        return false;
    }
    return true;
}

bool Application::InitializeInputProcessor()
{
    m_inputProcessor = std::make_unique<InputProcessor>(*m_configManager);
    
    
    return true;
}

int Application::Run()
{
    if (!m_initialized) {
        if (!Initialize()) {
            return 1;
        }
    }
    
    m_running = true;
    LOG_WRITE("Polling started... press Esc or close the window to quit.");
    
    // Main application loop
    while (m_running && m_windowManager->IsRunning()) {
        ProcessMessages();
        
        if (!m_running || !m_windowManager->IsRunning()) {
            break;
        }
        
        UpdateFrame();
        Sleep(FRAME_SLEEP_MS);
    }
    
    return 0;
}

void Application::ProcessMessages()
{
    while (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE)) {
        if (m_msg.message == WM_QUIT) {
            m_windowManager->SetRunning(false);
        }
        TranslateMessage(&m_msg);
        DispatchMessage(&m_msg);
    }
}

void Application::UpdateFrame()
{
    // Clear frame log for this frame
    Logger::GetInstance().ClearFrameLog();
    
    // Display gamepad information at the top
    if (m_directInputManager) {
        Logger::GetInstance().AppendGamepadInfo(
            m_directInputManager->IsDeviceConnected(),
            m_directInputManager->GetDeviceName().c_str(),
            m_directInputManager->GetDeviceInstanceName().c_str()
        );
    }
    
    ProcessGamepadInput();
    UpdateDisplay();
    CheckExitConditions();
}

void Application::ProcessGamepadInput()
{
    if (!m_directInputManager) {
        return;
    }

    if (m_directInputManager->IsDeviceConnected()) {
        if (m_directInputManager->PollAndGetState(m_joystickState)) {
            Logger::GetInstance().AppendState(m_joystickState);
            if (m_inputProcessor) {
                m_inputProcessor->ProcessGamepadInput(m_joystickState);
            }
        } else {
            LOG_WRITE("Gamepad disconnected. Waiting for reconnection...");
        }
    } else {
        Logger::GetInstance().AppendLog(L"Waiting for gamepad connection...");
        
        static DWORD last_retry_time = 0;
        DWORD current_time = GetTickCount();
        if (current_time - last_retry_time > 5000) { // Retry every 5 seconds
            m_directInputManager->TryToReconnect();
            last_retry_time = current_time;
        }
    }
}

void Application::UpdateDisplay()
{
    // Trigger WM_PAINT to update the screen
    InvalidateRect(m_windowManager->GetHwnd(), nullptr, TRUE);
    UpdateWindow(m_windowManager->GetHwnd());
}

void Application::CheckExitConditions()
{
    // Check for Esc key to exit
    SHORT esc = GetAsyncKeyState(VK_ESCAPE);
    if (esc & 0x8000) {
        PostMessage(m_windowManager->GetHwnd(), WM_CLOSE, 0, 0);
    }
}

void Application::Shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    m_running = false;
    CleanupResources();
    m_initialized = false;
    
    LOG_WRITE("Application shutdown completed.");
}

void Application::CleanupResources()
{
    // Clean up components in reverse order
    m_inputProcessor.reset();
    
    if (m_directInputManager) {
        m_directInputManager->Shutdown();
        m_directInputManager.reset();
    }
    
    m_windowManager.reset();
    m_configManager.reset();
    
    // Close logger
    Logger::GetInstance().Close();
}

std::string Application::GenerateLogPath() const
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    return (std::filesystem::path(exePath).parent_path() / "gamepad_mapper.log").string();
}