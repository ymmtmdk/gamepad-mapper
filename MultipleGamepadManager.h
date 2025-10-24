#pragma once
#include <windows.h>
#include <dinput.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// Forward declarations
class GamepadDevice;

// ComPtr alias for convenience
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

/**
 * @brief Multiple gamepad device management class
 * 
 * This class manages multiple gamepad devices simultaneously,
 * handling device enumeration, connection/disconnection, and
 * coordinated input processing.
 */
class MultipleGamepadManager {
public:
    // Constructor/Destructor (RAII)
    MultipleGamepadManager();
    ~MultipleGamepadManager();
    
    // Non-copyable
    MultipleGamepadManager(const MultipleGamepadManager&) = delete;
    MultipleGamepadManager& operator=(const MultipleGamepadManager&) = delete;
    
    // Initialization
    bool Initialize(HINSTANCE hInst, HWND hWnd);
    void Shutdown();
    
    // Device management
    void ScanForDevices();
    void ProcessAllDevices();
    bool TryToReconnectDevices();
    
    // Device access
    size_t GetDeviceCount() const { return m_devices.size(); }
    size_t GetConnectedDeviceCount() const;
    GamepadDevice* FindDeviceByName(const std::wstring& name) const;
    GamepadDevice* FindDeviceByGUID(const GUID& guid) const;
    
    // Device information
    std::vector<std::wstring> GetConnectedDeviceNames() const;
    std::vector<std::wstring> GetAllDeviceNames() const;
    
    // State queries
    bool IsInitialized() const { return m_initialized; }
    bool HasAnyConnectedDevices() const;

private:
    // Internal helpers
    bool CreateDirectInput(HINSTANCE hInst);
    void CleanupDisconnectedDevices();
    bool IsDeviceAlreadyManaged(const GUID& guid) const;
    
    // Device enumeration callback
    static BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
    
    // Member variables
    ComPtr<IDirectInput8> m_directInput;
    std::vector<std::unique_ptr<GamepadDevice>> m_devices;
    
    // Initialization state
    bool m_initialized;
    HWND m_hWnd;
    
    // Device tracking
    std::unordered_map<std::string, size_t> m_deviceIndexByGUID; // GUID string -> device index
    
    // Scan control
    DWORD m_lastScanTime;
    static constexpr DWORD SCAN_INTERVAL_MS = 5000; // Scan for new devices every 5 seconds
};