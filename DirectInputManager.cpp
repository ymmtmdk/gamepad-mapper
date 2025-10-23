#include "DirectInputManager.h"
#include "Logger.h"
#include <cstdio>


// DirectInputManager Class Implementation

DirectInputManager::DirectInputManager()
    : m_deviceConnected(false)
    , m_deviceAcquired(false)
    , m_initialized(false) {
}

DirectInputManager::~DirectInputManager() {
    Shutdown();
}

bool DirectInputManager::Initialize(HINSTANCE hInst, HWND hWnd) {
    if (m_initialized) {
        LOG_WRITE("DirectInputManager already initialized.");
        return true;
    }

    LOG_WRITE("Initializing DirectInputManager...");
    m_hWnd = hWnd; // Store hWnd for later use

    if (!CreateDirectInput(hInst)) {
        return false;
    }

    if (EnumerateDevices()) {
        ConfigureDevice(m_hWnd);
    } else {
        LOG_WRITE("No game controller found during initial scan. Will retry later.");
    }

    m_initialized = true;
    LOG_WRITE("DirectInputManager initialization finished (device may not be connected).");
    return true;
}

void DirectInputManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    LOG_WRITE("Shutting down DirectInputManager...");
    
    UnacquireDevice();
    
    // Reset device information
    m_deviceName.clear();
    m_deviceInstanceName.clear();
    m_deviceConnected = false;
    m_deviceAcquired = false;
    
    // COM smart pointers automatically release
    m_pDevice.Reset();
    m_pDirectInput.Reset();
    
    m_initialized = false;
    LOG_WRITE("DirectInputManager shutdown complete.");
}

bool DirectInputManager::CreateDirectInput(HINSTANCE hInst) {
    HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, 
                                   reinterpret_cast<void**>(m_pDirectInput.GetAddressOf()), nullptr);
    
    if (FAILED(hr)) {
        LOG_WRITE("DirectInput8Create failed. HRESULT: 0x%08X", hr);
        return false;
    }
    
    LOG_WRITE("DirectInput8 created successfully.");
    return true;
}

bool DirectInputManager::EnumerateDevices() {
    if (m_pDevice) {
        m_pDevice.Reset();
    }
    m_deviceConnected = false;
    m_deviceName.clear();
    m_deviceInstanceName.clear();

    HRESULT hr = m_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, 
                                           this, DIEDFL_ATTACHEDONLY);
    
    if (FAILED(hr)) {
        LOG_WRITE("EnumDevices failed. HRESULT: 0x%08X", hr);
        return false;
    }
    
    if (!m_deviceConnected) {
        // LOG_WRITE("No game controller found."); // Don't log this every time
        return false;
    }
    
    return true;
}

bool DirectInputManager::ConfigureDevice(HWND hWnd) {
    if (!m_pDevice) {
        LOG_WRITE("No device to configure.");
        return false;
    }

    // Set data format to joystick (DIJOYSTATE2)
    HRESULT hr = m_pDevice->SetDataFormat(&c_dfDIJoystick2);
    if (FAILED(hr)) {
        LOG_WRITE("SetDataFormat failed. HRESULT: 0x%08X", hr);
        return false;
    }

    // Set cooperative level
    hr = m_pDevice->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr)) {
        LOG_WRITE("SetCooperativeLevel failed. HRESULT: 0x%08X", hr);
        return false;
    }

    // Set axis ranges
    SetAxisRanges();

    // Acquire the device
    if (!AcquireDevice()) {
        LOG_WRITE("Initial device acquisition failed (may work in background).");
    }

    return true;
}

void DirectInputManager::SetAxisRanges() {
    if (!m_pDevice) return;

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
        m_pDevice->SetProperty(DIPROP_RANGE, &range.diph);
    }
    
    LOG_WRITE("Axis ranges set to [-1000, 1000].");
}

bool DirectInputManager::AcquireDevice() {
    if (!m_pDevice) {
        return false;
    }
    
    HRESULT hr = m_pDevice->Acquire();
    if (SUCCEEDED(hr)) {
        m_deviceAcquired = true;
        LOG_WRITE("Game controller acquired successfully.");
        return true;
    } else {
        m_deviceAcquired = false;
        LOG_WRITE("Device acquisition failed. HRESULT: 0x%08X", hr);
        return false;
    }
}

void DirectInputManager::UnacquireDevice() {
    if (m_pDevice && m_deviceAcquired) {
        m_pDevice->Unacquire();
        m_deviceAcquired = false;
        LOG_WRITE("Game controller unacquired.");
    }
}

bool DirectInputManager::PollAndGetState(DIJOYSTATE2& js) {
    if (!m_pDevice || !m_initialized) {
        m_deviceConnected = false;
        return false;
    }

    HRESULT hr = m_pDevice->Poll();
    if (FAILED(hr)) {
        hr = m_pDevice->Acquire();
        if (FAILED(hr)) {
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
                LOG_WRITE("Device lost or not acquired. Trying to find it again.");
                if (!TryToReconnect()) {
                    m_deviceConnected = false;
                }
            }
            return false;
        }
    }

    hr = m_pDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js);
    if (FAILED(hr)) {
        LOG_WRITE("GetDeviceState failed. HRESULT: 0x%08X", hr);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED || hr == DIERR_UNPLUGGED) {
             LOG_WRITE("Device is unplugged or lost.");
             m_deviceConnected = false;
             UnacquireDevice();
             m_pDevice.Reset();
        }
        return false;
    }

    return true;
}

bool DirectInputManager::TryToReconnect() {
    if (!m_initialized) return false;

    LOG_WRITE("Attempting to reconnect game controller...");
    
    if (EnumerateDevices()) {
        if (ConfigureDevice(m_hWnd)) {
            LOG_WRITE("Game controller reconnected successfully.");
            return true;
        } else {
            LOG_WRITE("Failed to configure reconnected device.");
            m_deviceConnected = false;
            return false;
        }
    }
    
    return false;
}

// Static callback for device enumeration
BOOL CALLBACK DirectInputManager::EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
    DirectInputManager* manager = reinterpret_cast<DirectInputManager*>(pContext);
    
    HRESULT hr = manager->m_pDirectInput->CreateDevice(pdidInstance->guidInstance, 
                                                      manager->m_pDevice.GetAddressOf(), nullptr);
    
    if (SUCCEEDED(hr)) {
        // Store device information
        manager->m_deviceName = pdidInstance->tszProductName;
        manager->m_deviceInstanceName = pdidInstance->tszInstanceName;
        manager->m_deviceConnected = true;
        
        LOG_WRITE_W(L"Game controller detected: %s (%s)", 
                   manager->m_deviceName.c_str(), 
                   manager->m_deviceInstanceName.c_str());
        
        return DIENUM_STOP;  // Use first found device
    }
    
    return DIENUM_CONTINUE;
}

