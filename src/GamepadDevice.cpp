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
    
    LOG_WRITE_W(L"Loading configuration for device: %s", m_deviceName.c_str());
    
    // Convert config file path to wide string safely for logging
    std::wstring configFilePathW;
    int pathSize = MultiByteToWideChar(CP_UTF8, 0, m_configFilePath.c_str(), -1, nullptr, 0);
    if (pathSize > 0) {
        configFilePathW.resize(pathSize - 1);
        MultiByteToWideChar(CP_UTF8, 0, m_configFilePath.c_str(), -1, &configFilePathW[0], pathSize);
    } else {
        configFilePathW = L"[path conversion failed]";
    }
    
    LOG_WRITE_W(L"Config file path: %s", configFilePathW.c_str());
    LOG_WRITE_W(L"Config file exists: %s", std::filesystem::exists(m_configFilePath) ? L"YES" : L"NO");
    
    // Create configuration manager with the device-specific path
    m_configManager = std::make_unique<JsonConfigManager>(m_configFilePath);
    
    // Try to load existing config
    bool loadResult = m_configManager->load();
    LOG_WRITE_W(L"Config load result: %s", loadResult ? L"SUCCESS" : L"FAILED");
    
    if (!loadResult) {
        // If loading fails, create a new default configuration file
        LOG_WRITE_W(L"Creating default configuration for device: %s", m_deviceName.c_str());
        if (!CreateConfigurationFile()) {
            LOG_WRITE_W(L"Failed to create new configuration for device: %s", m_deviceName.c_str());
            return false;
        }
        LOG_WRITE_W(L"Default configuration created successfully for device: %s", m_deviceName.c_str());
    } else {
        LOG_WRITE_W(L"Existing configuration loaded successfully for device: %s", m_deviceName.c_str());
        
        // Log some key configuration values to verify it's loaded correctly
        auto button0Keys = m_configManager->getButtonKeys(0);
        std::wstring button0Seq;
        for (size_t i = 0; i < button0Keys.size(); ++i) {
            if (i > 0) button0Seq += L"+";
            wchar_t buf[16];
            swprintf_s(buf, L"0x%02X", button0Keys[i]);
            button0Seq += buf;
        }
        LOG_WRITE_W(L"Button0 mapping: [%s] (threshold: %d)", 
                   button0Seq.c_str(), m_configManager->getStickThreshold());
    }
    
    // Convert config file path to wide string safely
    std::wstring finalConfigFilePathW;
    int finalSize = MultiByteToWideChar(CP_UTF8, 0, m_configFilePath.c_str(), -1, nullptr, 0);
    if (finalSize > 0) {
        finalConfigFilePathW.resize(finalSize - 1);
        MultiByteToWideChar(CP_UTF8, 0, m_configFilePath.c_str(), -1, &finalConfigFilePathW[0], finalSize);
    } else {
        finalConfigFilePathW = L"[conversion failed]";
    }
    
    LOG_WRITE_W(L"Configuration loaded for device: %s (file: %s)", 
               m_deviceName.c_str(), 
               finalConfigFilePathW.c_str());
    
    return true;
}

bool GamepadDevice::CreateConfigurationFile()
{
    // Get default configuration data
    auto [gamepadConfig, systemConfig] = JsonConfigManager::createDefaultConfig();

    // Set this data in the manager
    m_configManager->setConfig(gamepadConfig, systemConfig);

    // Save the new configuration to the device-specific file
    return m_configManager->save();
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
        
        // Process the input with device context
        m_inputProcessor->ProcessGamepadInput(m_currentState);
    }
}