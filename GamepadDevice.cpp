#include "GamepadDevice.h"
#include "JsonConfigManager.h"
#include "InputProcessor.h"
#include "Logger.h"
#include <algorithm>
#include <filesystem>

GamepadDevice::GamepadDevice()
    : m_connected(false)
    , m_acquired(false)
    , m_initialized(false)
    , m_currentState{}
    , m_deviceGUID{}
{
}

GamepadDevice::~GamepadDevice()
{
    Shutdown();
}

bool GamepadDevice::Initialize(IDirectInput8* pDirectInput, const DIDEVICEINSTANCE* deviceInstance, HWND hWnd)
{
    if (m_initialized) {
        return true;
    }
    
    // Store device information
    m_deviceName = deviceInstance->tszProductName;
    m_deviceInstanceName = deviceInstance->tszInstanceName;
    m_deviceGUID = deviceInstance->guidInstance;
    
    // Create device
    HRESULT hr = pDirectInput->CreateDevice(deviceInstance->guidInstance, m_device.GetAddressOf(), nullptr);
    if (FAILED(hr)) {
        LOG_WRITE_W(L"Failed to create device: %s. HRESULT: 0x%08X", m_deviceName.c_str(), hr);
        return false;
    }
    
    // Configure device
    if (!ConfigureDevice(hWnd)) {
        LOG_WRITE_W(L"Failed to configure device: %s", m_deviceName.c_str());
        return false;
    }
    
    // Load configuration
    if (!LoadConfiguration()) {
        LOG_WRITE_W(L"Failed to load configuration for device: %s", m_deviceName.c_str());
        return false;
    }
    
    // Initialize input processor
    m_inputProcessor = std::make_unique<InputProcessor>(*m_configManager);
    
    // Try to acquire device
    if (!AcquireDevice()) {
        LOG_WRITE_W(L"Initial device acquisition failed for: %s (may work in background)", m_deviceName.c_str());
    }
    
    m_initialized = true;
    m_connected = true;
    
    LOG_WRITE_W(L"GamepadDevice initialized successfully: %s (%s)", 
               m_deviceName.c_str(), m_deviceInstanceName.c_str());
    
    return true;
}

void GamepadDevice::Shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    LOG_WRITE_W(L"Shutting down GamepadDevice: %s", m_deviceName.c_str());
    
    UnacquireDevice();
    
    // Reset components
    m_inputProcessor.reset();
    m_configManager.reset();
    m_device.Reset();
    
    // Reset state
    m_connected = false;
    m_acquired = false;
    m_initialized = false;
    
    LOG_WRITE_W(L"GamepadDevice shutdown complete: %s", m_deviceName.c_str());
}

bool GamepadDevice::ConfigureDevice(HWND hWnd)
{
    if (!m_device) {
        return false;
    }
    
    // Set data format to joystick (DIJOYSTATE2)
    HRESULT hr = m_device->SetDataFormat(&c_dfDIJoystick2);
    if (FAILED(hr)) {
        LOG_WRITE("SetDataFormat failed. HRESULT: 0x%08X", hr);
        return false;
    }
    
    // Set cooperative level
    hr = m_device->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr)) {
        LOG_WRITE("SetCooperativeLevel failed. HRESULT: 0x%08X", hr);
        return false;
    }
    
    // Set axis ranges
    SetAxisRanges();
    
    return true;
}

void GamepadDevice::SetAxisRanges()
{
    if (!m_device) return;
    
    DIPROPRANGE range;
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYOFFSET;
    range.lMin = -1000;
    range.lMax = 1000;
    
    // Set ranges for all axes
    const DWORD axes[] = { DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ };
    
    for (DWORD axis : axes) {
        range.diph.dwObj = axis;
        m_device->SetProperty(DIPROP_RANGE, &range.diph);
    }
    
    LOG_WRITE_W(L"Axis ranges set to [-1000, 1000] for device: %s", m_deviceName.c_str());
}

std::string GamepadDevice::GetSafeFileName() const
{
    std::string safeName;
    
    // Convert wide string to narrow string
    int size = WideCharToMultiByte(CP_UTF8, 0, m_deviceName.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size > 0) {
        std::vector<char> buffer(size);
        WideCharToMultiByte(CP_UTF8, 0, m_deviceName.c_str(), -1, buffer.data(), size, nullptr, nullptr);
        safeName = buffer.data();
    }
    
    // Replace invalid filename characters
    std::string invalidChars = "\\/:*?\"<>|";
    for (char& c : safeName) {
        if (invalidChars.find(c) != std::string::npos) {
            c = '_';
        }
    }
    
    // Remove spaces and make it filesystem-safe
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    
    return safeName;
}

bool GamepadDevice::LoadConfiguration()
{
    // Generate config file path based on device name
    std::string safeDeviceName = GetSafeFileName();
    m_configFilePath = "gamepad_config_" + safeDeviceName + ".json";
    
    // Create configuration manager
    m_configManager = std::make_unique<JsonConfigManager>();
    
    // Try to load existing config, create default if not found
    if (!m_configManager->load(m_configFilePath)) {
        LOG_WRITE_W(L"Creating default configuration for device: %s", m_deviceName.c_str());
        if (!CreateConfigurationFile()) {
            return false;
        }
    }
    
    LOG_WRITE_W(L"Configuration loaded for device: %s (file: %s)", 
               m_deviceName.c_str(), 
               std::filesystem::path(m_configFilePath).filename().string().c_str());
    
    return true;
}

bool GamepadDevice::CreateConfigurationFile()
{
    // Create a default configuration and save it
    // The JsonConfigManager constructor will create default config
    // We just need to save it with our device-specific filename
    return m_configManager->save(m_configFilePath);
}

bool GamepadDevice::AcquireDevice()
{
    if (!m_device) {
        return false;
    }
    
    HRESULT hr = m_device->Acquire();
    if (SUCCEEDED(hr)) {
        m_acquired = true;
        LOG_WRITE_W(L"Device acquired successfully: %s", m_deviceName.c_str());
        return true;
    } else {
        m_acquired = false;
        LOG_WRITE_W(L"Device acquisition failed: %s. HRESULT: 0x%08X", m_deviceName.c_str(), hr);
        return false;
    }
}

void GamepadDevice::UnacquireDevice()
{
    if (m_device && m_acquired) {
        m_device->Unacquire();
        m_acquired = false;
        LOG_WRITE_W(L"Device unacquired: %s", m_deviceName.c_str());
    }
}

bool GamepadDevice::PollAndGetState()
{
    if (!m_device || !m_initialized) {
        m_connected = false;
        return false;
    }
    
    HRESULT hr = m_device->Poll();
    if (FAILED(hr)) {
        hr = m_device->Acquire();
        if (FAILED(hr)) {
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
                LOG_WRITE_W(L"Device lost or not acquired: %s. Trying to reconnect.", m_deviceName.c_str());
                m_connected = false;
            }
            return false;
        }
    }
    
    hr = m_device->GetDeviceState(sizeof(DIJOYSTATE2), &m_currentState);
    if (FAILED(hr)) {
        LOG_WRITE_W(L"GetDeviceState failed for device: %s. HRESULT: 0x%08X", m_deviceName.c_str(), hr);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED || hr == DIERR_UNPLUGGED) {
            LOG_WRITE_W(L"Device is unplugged or lost: %s", m_deviceName.c_str());
            m_connected = false;
            UnacquireDevice();
        }
        return false;
    }
    
    return true;
}

bool GamepadDevice::TryToReconnect(IDirectInput8* pDirectInput, HWND hWnd)
{
    if (!pDirectInput || m_connected) {
        return m_connected;
    }
    
    LOG_WRITE_W(L"Attempting to reconnect device: %s", m_deviceName.c_str());
    
    // Try to recreate the device
    HRESULT hr = pDirectInput->CreateDevice(m_deviceGUID, m_device.GetAddressOf(), nullptr);
    if (FAILED(hr)) {
        LOG_WRITE_W(L"Failed to recreate device: %s. HRESULT: 0x%08X", m_deviceName.c_str(), hr);
        return false;
    }
    
    // Reconfigure device
    if (!ConfigureDevice(hWnd)) {
        LOG_WRITE_W(L"Failed to reconfigure device: %s", m_deviceName.c_str());
        m_device.Reset();
        return false;
    }
    
    // Try to acquire
    if (AcquireDevice()) {
        m_connected = true;
        LOG_WRITE_W(L"Device reconnected successfully: %s", m_deviceName.c_str());
        return true;
    } else {
        LOG_WRITE_W(L"Failed to acquire reconnected device: %s", m_deviceName.c_str());
        return false;
    }
}

void GamepadDevice::ProcessInput()
{
    if (!m_inputProcessor || !m_connected) {
        return;
    }
    
    if (PollAndGetState()) {
        // Add device name prefix to frame log for identification
        FRAME_LOG_APPEND(L"[%s]", m_deviceName.c_str());
        
        // Log the current state for this device
        Logger::GetInstance().AppendState(m_currentState);
        
        // Process the input
        m_inputProcessor->ProcessGamepadInput(m_currentState);
    }
}