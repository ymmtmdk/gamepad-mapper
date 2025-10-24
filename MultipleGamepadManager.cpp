#include "MultipleGamepadManager.h"
#include "GamepadDevice.h"
#include "Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

MultipleGamepadManager::MultipleGamepadManager()
    : m_initialized(false)
    , m_hWnd(nullptr)
    , m_lastScanTime(0)
{
}

MultipleGamepadManager::~MultipleGamepadManager()
{
    Shutdown();
}

bool MultipleGamepadManager::Initialize(HINSTANCE hInst, HWND hWnd)
{
    if (m_initialized) {
        LOG_WRITE("MultipleGamepadManager already initialized.");
        return true;
    }
    
    LOG_WRITE("Initializing MultipleGamepadManager...");
    m_hWnd = hWnd;
    
    if (!CreateDirectInput(hInst)) {
        return false;
    }
    
    // Initial device scan
    ScanForDevices();
    
    m_initialized = true;
    LOG_WRITE("MultipleGamepadManager initialization completed. Found %zu devices.", m_devices.size());
    
    return true;
}

void MultipleGamepadManager::Shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    LOG_WRITE("Shutting down MultipleGamepadManager...");
    
    // Shutdown all devices
    for (auto& device : m_devices) {
        if (device) {
            device->Shutdown();
        }
    }
    
    // Clear containers
    m_devices.clear();
    m_deviceIndexByGUID.clear();
    
    // Release DirectInput
    m_directInput.Reset();
    
    m_initialized = false;
    LOG_WRITE("MultipleGamepadManager shutdown complete.");
}

bool MultipleGamepadManager::CreateDirectInput(HINSTANCE hInst)
{
    HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8,
                                   reinterpret_cast<void**>(m_directInput.GetAddressOf()), nullptr);
    
    if (FAILED(hr)) {
        LOG_WRITE("DirectInput8Create failed. HRESULT: 0x%08X", hr);
        return false;
    }
    
    LOG_WRITE("DirectInput8 created successfully for multiple gamepad management.");
    return true;
}

void MultipleGamepadManager::ScanForDevices()
{
    if (!m_directInput) {
        return;
    }
    
    LOG_WRITE("Scanning for gamepad devices...");
    
    // Clear the index map for rebuilding
    m_deviceIndexByGUID.clear();
    
    HRESULT hr = m_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback,
                                           this, DIEDFL_ATTACHEDONLY);
    
    if (FAILED(hr)) {
        LOG_WRITE("EnumDevices failed. HRESULT: 0x%08X", hr);
        return;
    }
    
    // Rebuild the index map
    for (size_t i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i]) {
            // Convert GUID to string for indexing
            const GUID& guid = m_devices[i]->GetGUID();
            std::ostringstream oss;
            oss << std::hex << std::uppercase
                << std::setfill('0') << std::setw(8) << guid.Data1 << "-"
                << std::setfill('0') << std::setw(4) << guid.Data2 << "-"
                << std::setfill('0') << std::setw(4) << guid.Data3 << "-";
            
            for (int j = 0; j < 8; ++j) {
                oss << std::setfill('0') << std::setw(2) << static_cast<int>(guid.Data4[j]);
                if (j == 1) oss << "-";
            }
            
            m_deviceIndexByGUID[oss.str()] = i;
        }
    }
    
    // Clean up any devices that are no longer connected
    CleanupDisconnectedDevices();
    
    LOG_WRITE("Device scan completed. Managing %zu devices.", m_devices.size());
    m_lastScanTime = GetTickCount();
}

void MultipleGamepadManager::CleanupDisconnectedDevices()
{
    // Remove devices that are no longer connected
    auto it = std::remove_if(m_devices.begin(), m_devices.end(),
        [](const std::unique_ptr<GamepadDevice>& device) {
            return !device || !device->IsConnected();
        });
    
    if (it != m_devices.end()) {
        size_t removedCount = std::distance(it, m_devices.end());
        LOG_WRITE("Removing %zu disconnected devices.", removedCount);
        m_devices.erase(it, m_devices.end());
    }
}

bool MultipleGamepadManager::IsDeviceAlreadyManaged(const GUID& guid) const
{
    return std::any_of(m_devices.begin(), m_devices.end(),
        [&guid](const std::unique_ptr<GamepadDevice>& device) {
            return device && IsEqualGUID(device->GetGUID(), guid);
        });
}

void MultipleGamepadManager::ProcessAllDevices()
{
    if (!m_initialized) {
        return;
    }
    
    // Periodically scan for new devices
    DWORD currentTime = GetTickCount();
    if (currentTime - m_lastScanTime > SCAN_INTERVAL_MS) {
        ScanForDevices();
    }
    
    // Process input from all connected devices
    for (auto& device : m_devices) {
        if (device && device->IsConnected()) {
            device->ProcessInput();
        }
    }
    
    // Try to reconnect any disconnected devices
    TryToReconnectDevices();
}

bool MultipleGamepadManager::TryToReconnectDevices()
{
    bool anyReconnected = false;
    
    for (auto& device : m_devices) {
        if (device && !device->IsConnected()) {
            if (device->TryToReconnect(m_directInput.Get(), m_hWnd)) {
                anyReconnected = true;
            }
        }
    }
    
    return anyReconnected;
}

size_t MultipleGamepadManager::GetConnectedDeviceCount() const
{
    return std::count_if(m_devices.begin(), m_devices.end(),
        [](const std::unique_ptr<GamepadDevice>& device) {
            return device && device->IsConnected();
        });
}

GamepadDevice* MultipleGamepadManager::FindDeviceByName(const std::wstring& name) const
{
    auto it = std::find_if(m_devices.begin(), m_devices.end(),
        [&name](const std::unique_ptr<GamepadDevice>& device) {
            return device && device->GetName() == name;
        });
    
    return (it != m_devices.end()) ? it->get() : nullptr;
}

GamepadDevice* MultipleGamepadManager::FindDeviceByGUID(const GUID& guid) const
{
    auto it = std::find_if(m_devices.begin(), m_devices.end(),
        [&guid](const std::unique_ptr<GamepadDevice>& device) {
            return device && IsEqualGUID(device->GetGUID(), guid);
        });
    
    return (it != m_devices.end()) ? it->get() : nullptr;
}

std::vector<std::wstring> MultipleGamepadManager::GetConnectedDeviceNames() const
{
    std::vector<std::wstring> names;
    
    for (const auto& device : m_devices) {
        if (device && device->IsConnected()) {
            names.push_back(device->GetName());
        }
    }
    
    return names;
}

std::vector<std::wstring> MultipleGamepadManager::GetAllDeviceNames() const
{
    std::vector<std::wstring> names;
    
    for (const auto& device : m_devices) {
        if (device) {
            names.push_back(device->GetName());
        }
    }
    
    return names;
}

bool MultipleGamepadManager::HasAnyConnectedDevices() const
{
    return std::any_of(m_devices.begin(), m_devices.end(),
        [](const std::unique_ptr<GamepadDevice>& device) {
            return device && device->IsConnected();
        });
}

// Static callback for device enumeration
BOOL CALLBACK MultipleGamepadManager::EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    MultipleGamepadManager* manager = reinterpret_cast<MultipleGamepadManager*>(pContext);
    
    // Check if this device is already managed
    if (manager->IsDeviceAlreadyManaged(pdidInstance->guidInstance)) {
        return DIENUM_CONTINUE; // Already have this device, continue enumeration
    }
    
    // Create a new GamepadDevice
    auto newDevice = std::make_unique<GamepadDevice>();
    
    if (newDevice->Initialize(manager->m_directInput.Get(), pdidInstance, manager->m_hWnd)) {
        LOG_WRITE_W(L"New gamepad device added: %s (%s)",
                   newDevice->GetName().c_str(),
                   newDevice->GetInstanceName().c_str());
        
        manager->m_devices.push_back(std::move(newDevice));
    } else {
        LOG_WRITE_W(L"Failed to initialize gamepad device: %s",
                   pdidInstance->tszProductName);
    }
    
    return DIENUM_CONTINUE; // Continue enumeration to find all devices
}