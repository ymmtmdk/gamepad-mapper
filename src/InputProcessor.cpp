#include "InputProcessor.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "DisplayBuffer.h"
#include <cstring>
#include <algorithm>


// =====================================
// InputProcessor Class Implementation
// =====================================

InputProcessor::InputProcessor() 
    : m_prevPOV(0xFFFFFFFF)
    , m_configManager(nullptr)
    , m_displayBuffer(nullptr)
{
    InitializeState();
}

InputProcessor::InputProcessor(const ConfigManager& config)
    : m_prevPOV(0xFFFFFFFF)
    , m_configManager(&config)
    , m_displayBuffer(nullptr)
{
    InitializeState();
}

InputProcessor::InputProcessor(const ConfigManager& config, DisplayBuffer* displayBuffer)
    : m_prevPOV(0xFFFFFFFF)
    , m_configManager(&config)
    , m_displayBuffer(displayBuffer)
{
    InitializeState();
}

void InputProcessor::SetConfig(const ConfigManager& config)
{
    m_configManager = &config;
}

void InputProcessor::InitializeState()
{
    ResetState();
}

void InputProcessor::ResetState()
{
    m_prevButtons.fill(false);
    m_prevPOV = 0xFFFFFFFF;
    m_prevAxisDown.fill(false);
}

void InputProcessor::SendVirtualKeySequence(const std::vector<WORD>& vks, bool down)
{
    if (vks.empty()) return;
    
    auto inputs = CreateKeyInputSequence(vks, down);
    SendInputSequence(inputs);
    
    // ログ出力（シーケンス表示）
    std::wstring seq;
    for (size_t i = 0; i < vks.size(); ++i) {
        wchar_t buf[32];
        swprintf_s(buf, L"0x%02X", vks[i]);
        if (i) seq += L"+";
        seq += buf;
    }
    // Display input sequence information
    if (m_displayBuffer) {
        m_displayBuffer->AddFormattedLine(L"SendInputSeq: %s %s", seq.c_str(), down ? L"DOWN" : L"UP");
    }
}

void InputProcessor::SendVirtualKey(WORD vk, bool down)
{
    if (vk == 0) return;
    std::vector<WORD> seq = { vk };
    SendVirtualKeySequence(seq, down);
}

std::vector<INPUT> InputProcessor::CreateKeyInputSequence(const std::vector<WORD>& vks, bool down)
{
    size_t n = vks.size();
    std::vector<INPUT> inputs;
    inputs.reserve(n);
    
    if (down) {
        // Press keys in order
        for (size_t i = 0; i < n; ++i) {
            INPUT ip{};
            ip.type = INPUT_KEYBOARD;
            ip.ki.wVk = vks[i];
            ip.ki.dwFlags = 0;
            inputs.push_back(ip);
        }
    } else {
        // Release keys in reverse order
        for (size_t i = 0; i < n; ++i) {
            INPUT ip{};
            ip.type = INPUT_KEYBOARD;
            ip.ki.wVk = vks[n - 1 - i];
            ip.ki.dwFlags = KEYEVENTF_KEYUP;
            inputs.push_back(ip);
        }
    }
    
    return inputs;
}

void InputProcessor::SendInputSequence(const std::vector<INPUT>& inputs)
{
    if (!inputs.empty()) {
        SendInput(static_cast<UINT>(inputs.size()), 
                 const_cast<INPUT*>(inputs.data()), 
                 sizeof(INPUT));
    }
}

void InputProcessor::ProcessGamepadInput(const DIJOYSTATE2& js)
{
    if (!m_configManager) {
        // No configuration available, cannot process input
        return;
    }
    
    ProcessButtons(js);
    ProcessPOV(js);
    ProcessAnalogSticks(js);
}

void InputProcessor::ProcessButtons(const DIJOYSTATE2& js)
{
    // Process all buttons with mappings
    for (size_t i = 0; i < MAX_BUTTONS; ++i) {
        const auto& vks = m_configManager->getButtonKeys(static_cast<int>(i));
        if (vks.empty()) continue;
        
        bool pressed = (js.rgbButtons[i] & 0x80) != 0;
        if (pressed != m_prevButtons[i]) {
            ProcessButtonInternal(i, pressed);
            m_prevButtons[i] = pressed;
        }
    }
}

void InputProcessor::ProcessButtonInternal(size_t buttonIndex, bool pressed)
{
    const auto& vks = m_configManager->getButtonKeys(static_cast<int>(buttonIndex));
    
    // Log detailed button event with configuration info
    std::wstring vkSeq;
    for (size_t i = 0; i < vks.size(); ++i) {
        if (i > 0) vkSeq += L"+";
        wchar_t buf[16];
        swprintf_s(buf, L"0x%02X", vks[i]);
        vkSeq += buf;
    }
    
    std::wstring configPath;
    try {
        configPath = m_configManager->getConfigFilePath();
    } catch (...) {
        configPath = L"[config path error]";
    }
    LOG_DEBUG_W(L"Button" + std::to_wstring(buttonIndex) + L" -> Keys[" + vkSeq + L"] " + 
               (pressed ? L"PRESSED" : L"RELEASED") + L" (Config: " + configPath + L")");
    
    // Display button event information
    if (m_displayBuffer) {
        m_displayBuffer->AddFormattedLine(L"Button%zu -> Keys[%s] %s", 
                        buttonIndex, 
                        vkSeq.c_str(), 
                        pressed ? L"PRESSED" : L"RELEASED");
    }
    
    SendVirtualKeySequence(vks, pressed);
}

void InputProcessor::ProcessPOV(const DIJOYSTATE2& js)
{
    // POV (Hat) -> Map to configured directions
    DWORD pov = js.rgdwPOV[0];
    bool up = false, down = false, left = false, right = false;
    
    if (pov != 0xFFFFFFFF) {
        if (pov <= 4500 || pov >= 31500) up = true;
        if (pov >= 4500 && pov <= 13500) right = true;
        if (pov >= 13500 && pov <= 22500) down = true;
        if (pov >= 22500 && pov <= 31500) left = true;
    }
    
    // Process each direction if it has changed
    if (up != m_prevAxisDown[AX_UP]) {
        ProcessPOVDirection(AX_UP, up);
        m_prevAxisDown[AX_UP] = up;
    }
    if (down != m_prevAxisDown[AX_DOWN]) {
        ProcessPOVDirection(AX_DOWN, down);
        m_prevAxisDown[AX_DOWN] = down;
    }
    if (left != m_prevAxisDown[AX_LEFT]) {
        ProcessPOVDirection(AX_LEFT, left);
        m_prevAxisDown[AX_LEFT] = left;
    }
    if (right != m_prevAxisDown[AX_RIGHT]) {
        ProcessPOVDirection(AX_RIGHT, right);
        m_prevAxisDown[AX_RIGHT] = right;
    }
    
    m_prevPOV = pov;
}

void InputProcessor::ProcessPOVDirection(size_t direction, bool active)
{
    std::string dirStr;
    switch (direction) {
        case AX_UP: dirStr = "up"; break;
        case AX_DOWN: dirStr = "down"; break;
        case AX_LEFT: dirStr = "left"; break;
        case AX_RIGHT: dirStr = "right"; break;
        default: return;
    }

    const auto& vks = m_configManager->getDpadKeys(dirStr);
    if (!vks.empty()) {
        // Log detailed POV event with configuration info
        std::wstring vkSeq;
        for (size_t i = 0; i < vks.size(); ++i) {
            if (i > 0) vkSeq += L"+";
            wchar_t buf[16];
            swprintf_s(buf, L"0x%02X", vks[i]);
            vkSeq += buf;
        }
        
        const wchar_t* dirName = (direction == AX_UP) ? L"Up" : 
                                (direction == AX_DOWN) ? L"Down" :
                                (direction == AX_LEFT) ? L"Left" : L"Right";
        
        std::wstring configPath;
        try {
            configPath = m_configManager->getConfigFilePath();
        } catch (...) {
            configPath = L"[config path error]";
        }
        LOG_DEBUG_W(L"POV " + std::wstring(dirName) + L" -> Keys[" + vkSeq + L"] " + 
                   (active ? L"ON" : L"OFF") + L" (Config: " + configPath + L")");
        
        // Display POV event information
        if (m_displayBuffer) {
            m_displayBuffer->AddFormattedLine(L"POV %s -> Keys[%s] %s", dirName, vkSeq.c_str(), active ? L"ON" : L"OFF");
        }
        
        SendVirtualKeySequence(vks, active);
    }
}

void InputProcessor::ProcessAnalogSticks(const DIJOYSTATE2& js)
{
    // Left stick (X/Y) -> Map to configured keys with threshold
    LONG threshold = m_configManager->getStickThreshold();
    bool stickLeft = js.lX < -threshold;
    bool stickRight = js.lX > threshold;
    bool stickUp = js.lY < -threshold;    // Up is negative direction
    bool stickDown = js.lY > threshold;

    // Process each axis direction if it has changed
    if (stickLeft != m_prevAxisDown[AX_LEFT]) {
        ProcessAxisDirection(AX_LEFT, stickLeft);
        m_prevAxisDown[AX_LEFT] = stickLeft;
    }
    if (stickRight != m_prevAxisDown[AX_RIGHT]) {
        ProcessAxisDirection(AX_RIGHT, stickRight);
        m_prevAxisDown[AX_RIGHT] = stickRight;
    }
    if (stickUp != m_prevAxisDown[AX_UP]) {
        ProcessAxisDirection(AX_UP, stickUp);
        m_prevAxisDown[AX_UP] = stickUp;
    }
    if (stickDown != m_prevAxisDown[AX_DOWN]) {
        ProcessAxisDirection(AX_DOWN, stickDown);
        m_prevAxisDown[AX_DOWN] = stickDown;
    }
}

void InputProcessor::ProcessAxisDirection(size_t direction, bool active)
{
    std::string dirStr;
    switch (direction) {
        case AX_UP: dirStr = "up"; break;
        case AX_DOWN: dirStr = "down"; break;
        case AX_LEFT: dirStr = "left"; break;
        case AX_RIGHT: dirStr = "right"; break;
        default: return;
    }

    const auto& vks = m_configManager->getStickKeys(dirStr);
    if (!vks.empty()) {
        // Log detailed axis event with configuration info
        std::wstring vkSeq;
        for (size_t i = 0; i < vks.size(); ++i) {
            if (i > 0) vkSeq += L"+";
            wchar_t buf[16];
            swprintf_s(buf, L"0x%02X", vks[i]);
            vkSeq += buf;
        }
        
        const wchar_t* dirName = (direction == AX_UP) ? L"Up" : 
                                (direction == AX_DOWN) ? L"Down" :
                                (direction == AX_LEFT) ? L"Left" : L"Right";
        
        std::wstring configPath;
        try {
            configPath = m_configManager->getConfigFilePath();
        } catch (...) {
            configPath = L"[config path error]";
        }
        LOG_DEBUG_W(L"Axis " + std::wstring(dirName) + L" -> Keys[" + vkSeq + L"] " + 
                   (active ? L"ON" : L"OFF") + L" (Config: " + configPath + L")");
        
        // Display axis event information
        if (m_displayBuffer) {
            m_displayBuffer->AddFormattedLine(L"Axis %s -> Keys[%s] %s", dirName, vkSeq.c_str(), active ? L"ON" : L"OFF");
        }
        
        SendVirtualKeySequence(vks, active);
    }
}

