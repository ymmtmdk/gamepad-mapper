#pragma once
#include <windows.h>
#include <dinput.h>
#include <vector>
#include <array>
#include <memory>
#include "Constants.h"

// Forward declarations
class ConfigManager;
class DisplayBuffer;

class InputProcessor {
public:
    // Constructor/Destructor (RAII)
    InputProcessor();
    explicit InputProcessor(const ConfigManager& config);
    InputProcessor(const ConfigManager& config, DisplayBuffer* displayBuffer);
    ~InputProcessor() = default;
    
    // Non-copyable, but movable
    InputProcessor(const InputProcessor&) = delete;
    InputProcessor& operator=(const InputProcessor&) = delete;
    InputProcessor(InputProcessor&&) = default;
    InputProcessor& operator=(InputProcessor&&) = default;
    
    // Configuration management
    void SetConfig(const ConfigManager& config);
    const ConfigManager* GetConfig() const { return m_configManager; }
    
    // Display management
    void SetDisplayBuffer(DisplayBuffer* displayBuffer) { m_displayBuffer = displayBuffer; }
    
    // State management
    void InitializeState();
    void ResetState();
    
    // Main processing method
    void ProcessGamepadInput(const DIJOYSTATE2& js);
    
    // Individual input type processors
    void ProcessButtons(const DIJOYSTATE2& js);
    void ProcessPOV(const DIJOYSTATE2& js);
    void ProcessAnalogSticks(const DIJOYSTATE2& js);
    
    // Key sending methods
    void SendVirtualKey(WORD vk, bool down);
    void SendVirtualKeySequence(const std::vector<WORD>& vks, bool down);

private:
    // Internal state management
    static constexpr size_t MAX_BUTTONS = AppConstants::MAX_BUTTONS;
    static constexpr size_t AXIS_DIRECTIONS = AppConstants::AXIS_DIRECTIONS;
    
    // State tracking (encapsulated)
    std::array<bool, MAX_BUTTONS> m_prevButtons;
    DWORD m_prevPOV;
    std::array<bool, AXIS_DIRECTIONS> m_prevAxisDown; // 0: left, 1: right, 2: up, 3: down
    
    // Configuration reference
    const ConfigManager* m_configManager;
    
    // Display buffer for screen output (not logging)
    DisplayBuffer* m_displayBuffer;
    
    // Helper methods
    void ProcessButtonInternal(size_t buttonIndex, bool pressed);
    void ProcessPOVDirection(size_t direction, bool active);
    void ProcessAxisDirection(size_t direction, bool active);
    void SendInputSequence(const std::vector<INPUT>& inputs);
    std::vector<INPUT> CreateKeyInputSequence(const std::vector<WORD>& vks, bool down);
};

