#pragma once
#include <windows.h>
#include <dinput.h>
#include <wrl/client.h>
#include <string>
#include <memory>

// ComPtr alias for convenience
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class DirectInputManager {
public:
    // Constructor/Destructor (RAII)
    DirectInputManager();
    ~DirectInputManager();
    
    // Non-copyable
    DirectInputManager(const DirectInputManager&) = delete;
    DirectInputManager& operator=(const DirectInputManager&) = delete;
    
    // Initialization
    bool Initialize(HINSTANCE hInst, HWND hWnd);
    void Shutdown();
    
    // Device access
    IDirectInputDevice8* GetDevice() const { return m_pDevice.Get(); }
    bool IsDeviceConnected() const { return m_deviceConnected; }
    
    // Device information
    const std::wstring& GetDeviceName() const { return m_deviceName; }
    const std::wstring& GetDeviceInstanceName() const { return m_deviceInstanceName; }
    
    // Device operations
    bool AcquireDevice();
    void UnacquireDevice();
    bool PollAndGetState(DIJOYSTATE2& js);
    bool TryToReconnect();

private:
    // Internal initialization helpers
    bool CreateDirectInput(HINSTANCE hInst);
    bool EnumerateDevices();
    bool ConfigureDevice(HWND hWnd);
    void SetAxisRanges();
    
    // Static callback for device enumeration
    static BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
    
    // Member variables (encapsulated state)
    ComPtr<IDirectInput8> m_pDirectInput;
    ComPtr<IDirectInputDevice8> m_pDevice;
    
    // Device information
    std::wstring m_deviceName;
    std::wstring m_deviceInstanceName;
    bool m_deviceConnected;
    bool m_deviceAcquired;
    
    // Initialization state
    bool m_initialized;
    HWND m_hWnd;
};

