#include "Application.h"
#include "WindowManager.h"
#include "MultipleGamepadManager.h"
#include "GamepadDevice.h"
#include "Logger.h"
#include "ModernLogger.h"
#include "DisplayBuffer.h"
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <algorithm>

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_running(false)
    , m_initialized(false)
    , m_msg{}
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
        
        if (!InitializeWindow()) {
            CleanupResources();
            throw std::runtime_error("Window initialization failed");
        }
        
        if (!InitializeGamepadManager()) {
            CleanupResources();
            throw std::runtime_error("Gamepad Manager initialization failed");
        }
        
        m_initialized = true;
        LOG_INFO("Application initialization completed successfully with multiple gamepad support.");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Initialization failed: {}", e.what());
        CleanupResources();
        return false;
    }
}

bool Application::InitializeLogger()
{
    std::string logPath = GenerateLogPath();
    if (!ModernLogger::GetInstance().Init(logPath)) {
        MessageBox(nullptr, L"Failed to initialize log file!", L"Error", MB_ICONERROR);
        return false;
    }

    // Initialize display buffer for screen output
    m_displayBuffer = std::make_unique<DisplayBuffer>(150); // Allow more lines for better debugging
    m_displayBuffer->SetTimestampEnabled(false); // Keep display clean
    m_displayBuffer->SetAutoSeparator(true);
    return true;
}

bool Application::InitializeWindow()
{
    m_windowManager = std::make_unique<WindowManager>(m_hInstance, L"Gamepad Mapper", m_displayBuffer.get());
    if (!m_windowManager->Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        MessageBox(nullptr, L"Window initialization failed!", L"Error", MB_ICONERROR);
        return false;
    }
    return true;
}

bool Application::InitializeGamepadManager()
{
    m_gamepadManager = std::make_unique<MultipleGamepadManager>();
    
    // Inject display buffer dependency
    m_gamepadManager->SetDisplayBuffer(m_displayBuffer.get());
    
    if (!m_gamepadManager->Initialize(m_hInstance, m_windowManager->GetHwnd())) {
        // This will only fail in case of a fatal error, like DirectInput8Create failing.
        MessageBox(nullptr, L"A critical error occurred while initializing Multiple Gamepad Manager.", L"Fatal Error", MB_ICONERROR);
        return false;
    }
    
    // Log initial gamepad status
    LogGamepadStatus();
    
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
    LOG_INFO("Multi-gamepad polling started... press Esc or close the window to quit.");
    
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
    m_displayBuffer->Clear();
    
    // Display gamepad information at the top
    LogGamepadStatus();
    
    ProcessGamepadInput();
    UpdateDisplay();
    CheckExitConditions();
}

void Application::LogGamepadStatus()
{
    if (!m_gamepadManager) {
        return;
    }
    
    size_t totalDevices = m_gamepadManager->GetDeviceCount();
    size_t connectedDevices = m_gamepadManager->GetConnectedDeviceCount();
    
    if (totalDevices == 0) {
        m_displayBuffer->AddLine(L"No gamepad devices found. Scanning for devices...");
    } else {
        // Log summary using DisplayBuffer for formatted output
        m_displayBuffer->AddFormattedLine(L"Gamepad Status: %zu/%zu devices connected", 
                                         connectedDevices, totalDevices);
        
        // Log individual device status
        auto connectedNames = m_gamepadManager->GetConnectedDeviceNames();
        for (const auto& name : connectedNames) {
            m_displayBuffer->AddFormattedLine(L"  Connected: %s", name.c_str());
        }
        
        // If there are disconnected devices, show them too
        auto allNames = m_gamepadManager->GetAllDeviceNames();
        for (const auto& name : allNames) {
            // Check if this device is in the connected list
            auto it = std::find(connectedNames.begin(), connectedNames.end(), name);
            bool isConnected = (it != connectedNames.end());
            if (!isConnected) {
                m_displayBuffer->AddFormattedLine(L"  • Disconnected: %s", name.c_str());
            }
        }
    }
}

void Application::ProcessGamepadInput()
{
    if (!m_gamepadManager) {
        return;
    }
    
    if (m_gamepadManager->HasAnyConnectedDevices()) {
        // Process all connected devices
        m_gamepadManager->ProcessAllDevices();
    } else {
        // No devices connected, show waiting message
        m_displayBuffer->AddLine(L"Waiting for gamepad connections...");
        
        // The gamepad manager will automatically scan for new devices periodically
        // We just need to call ProcessAllDevices to trigger the scan
        m_gamepadManager->ProcessAllDevices();
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
    
    LOG_INFO("Application shutdown completed.");
}

void Application::CleanupResources()
{
    // Clean up components in reverse order
    if (m_gamepadManager) {
        m_gamepadManager->Shutdown();
        m_gamepadManager.reset();
    }
    
    m_windowManager.reset();
    
    // Close logger
    ModernLogger::GetInstance().Close();
}

std::string Application::GenerateLogPath() const
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    return (std::filesystem::path(exePath).parent_path() / "multi_gamepad_mapper.log").string();
}
