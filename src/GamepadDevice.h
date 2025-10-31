#pragma once
#include <windows.h>
#include <dinput.h>
#include <wrl/client.h>
#include <string>
#include <memory>

// Forward declarations
class JsonConfigManager;
class InputProcessor;
class DisplayBuffer;

// ComPtr alias for convenience
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

/**
 * @brief Individual gamepad device management class
 * 
 * This class encapsulates a single gamepad device with its own configuration,
 * input processor, and state management.
 */
class GamepadDevice {
public:
    // Constructor/Destructor (RAII)
    GamepadDevice();
    ~GamepadDevice();
    
    // Non-copyable but movable
    GamepadDevice(const GamepadDevice&) = delete;
    GamepadDevice& operator=(const GamepadDevice&) = delete;
    GamepadDevice(GamepadDevice&&) = default;
    GamepadDevice& operator=(GamepadDevice&&) = default;
    
    // Initialization
    bool Initialize(IDirectInput8* pDirectInput, const DIDEVICEINSTANCE* deviceInstance, HWND hWnd);
    void Shutdown();
    
    // Device information
    const std::wstring& GetName() const { return m_deviceName; }
    const std::wstring& GetInstanceName() const { return m_deviceInstanceName; }
    const GUID& GetGUID() const { return m_deviceGUID; }
    std::string GetSafeFileName() const;
    
    // Device state
    bool IsConnected() const { return m_connected; }
    bool IsAcquired() const { return m_acquired; }
    
    // Device operations
    bool AcquireDevice();
    void UnacquireDevice();
    bool PollAndGetState();
    bool TryToReconnect(IDirectInput8* pDirectInput, HWND hWnd);
    
    // Input processing
    void ProcessInput();
    
    // Display buffer injection
    void SetDisplayBuffer(DisplayBuffer* displayBuffer) { m_displayBuffer = displayBuffer; }
    
    // Configuration management
    bool LoadConfiguration();
    const JsonConfigManager* GetConfig() const { return m_configManager.get(); }

private:
    // Internal initialization helpers
    bool ConfigureDevice(HWND hWnd);
    void SetAxisRanges();
    bool CreateConfigurationFile();
    
    // Device components
    ComPtr<IDirectInputDevice8> m_device;
    std::unique_ptr<JsonConfigManager> m_configManager;
    std::unique_ptr<InputProcessor> m_inputProcessor;
    
    // Device information
    std::wstring m_deviceName;
    std::wstring m_deviceInstanceName;
    GUID m_deviceGUID;
    
    // Device state
    bool m_connected;
    bool m_acquired;
    bool m_initialized;
    DIJOYSTATE2 m_currentState;
    
    // Dependencies
    DisplayBuffer* m_displayBuffer = nullptr;
    
    // Configuration
    std::string m_configFilePath;
};