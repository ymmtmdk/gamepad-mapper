#include "DisplayBuffer.h"
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <iomanip>
#include <sstream>
#include <algorithm>

DisplayBuffer::DisplayBuffer(size_t maxLines)
    : m_maxLines(std::clamp(maxLines, MIN_MAX_LINES, MAX_MAX_LINES))
    , m_totalLinesAdded(0)
{
    m_lines.reserve(m_maxLines + 10); // Reserve some extra space
}

void DisplayBuffer::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lines.clear();
}

void DisplayBuffer::SetMaxLines(size_t maxLines) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxLines = std::clamp(maxLines, MIN_MAX_LINES, MAX_MAX_LINES);
    TrimToMaxLines();
}

size_t DisplayBuffer::GetMaxLines() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_maxLines;
}

void DisplayBuffer::AddLine(const std::wstring& line) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AddLineInternal(line);
}

void DisplayBuffer::AddFormattedLine(const wchar_t* fmt, ...) {
    wchar_t buffer[512];
    va_list args;
    va_start(args, fmt);
    vswprintf_s(buffer, _countof(buffer), fmt, args);
    va_end(args);
    
    AddLine(buffer);
}

void DisplayBuffer::AddGamepadHeader(const std::wstring& deviceName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_lines.empty()) {
        AddLineInternal(L"");
    }
    
    AddLineInternal(L"=== gamepad ===");
    AddLineInternal(L"name: " + deviceName);
}

void DisplayBuffer::AddGamepadInfo(bool connected, 
                                  const std::wstring& productName, 
                                  const std::wstring& instanceName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_lines.empty()) {
        AddLineInternal(L"");
    }
    
    AddLineInternal(L"=== gamepad ===");
    
    if (connected) {
        AddLineInternal(L"name: " + (productName.empty() ? L"Unknown" : productName));
        AddLineInternal(L"instance name: " + (instanceName.empty() ? L"Unknown" : instanceName));
        AddLineInternal(L"status: connected");
    } else {
        AddLineInternal(L"status: not connected");
    }
    
}

void DisplayBuffer::AddGamepadState(const std::wstring& deviceName, 
                                   const DIJOYSTATE2& state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Add device name prefix
    AddLineInternal(L"[" + deviceName + L"]");
    
    // Add axes information
    wchar_t buffer[256];
    swprintf_s(buffer, L"Axes: X=%ld Y=%ld Z=%ld RX=%ld RY=%ld RZ=%ld", 
               state.lX, state.lY, state.lZ, state.lRx, state.lRy, state.lRz);
    AddLineInternal(buffer);
    
    // Add slider information
    swprintf_s(buffer, L"Sliders: S0=%ld S1=%ld", state.rglSlider[0], state.rglSlider[1]);
    AddLineInternal(buffer);
    
    // Add POV information
    for (int i = 0; i < 4; ++i) {
        DWORD pov = state.rgdwPOV[i];
        if (pov == 0xFFFFFFFF) {
            swprintf_s(buffer, L"POV%d: -", i);
        } else {
            swprintf_s(buffer, L"POV%d: %lu", i, pov);
        }
        AddLineInternal(buffer);
    }
    
    // Add button information (most important for debugging)
    std::wstring buttonStr = L"Btns:";
    for (int i = 0; i < 32; ++i) {
        buttonStr.push_back((state.rgbButtons[i] & 0x80) ? L'1' : L'0');
        if ((i + 1) % 8 == 0 && i + 1 < 32) buttonStr.push_back(L' ');
    }
    AddLineInternal(buttonStr);
}

void DisplayBuffer::AddStatusLine(const std::wstring& status) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AddLineInternal(L"Status: " + status);
}

void DisplayBuffer::AddSeparator() {
    std::lock_guard<std::mutex> lock(m_mutex);
    AddLineInternal(L"");
}

const std::vector<std::wstring>& DisplayBuffer::GetLines() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lines;
}

size_t DisplayBuffer::GetLineCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lines.size();
}

bool DisplayBuffer::IsEmpty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lines.empty();
}

size_t DisplayBuffer::GetTotalLinesAdded() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_totalLinesAdded;
}

void DisplayBuffer::ResetStatistics() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_totalLinesAdded = 0;
}

// Private helper methods
void DisplayBuffer::AddLineInternal(const std::wstring& line) {
    std::wstring finalLine = line;
    
    m_lines.push_back(finalLine);
    m_totalLinesAdded++;
    
    TrimToMaxLines();
}

void DisplayBuffer::TrimToMaxLines() {
    if (m_lines.size() > m_maxLines) {
        size_t excessLines = m_lines.size() - m_maxLines;
        m_lines.erase(m_lines.begin(), m_lines.begin() + excessLines);
    }
}

std::wstring DisplayBuffer::FormatGamepadState(const DIJOYSTATE2& state) {
    std::wostringstream oss;
    oss << L"X:" << state.lX 
        << L" Y:" << state.lY 
        << L" Btns:";
    
    for (int i = 0; i < 8; ++i) {
        oss << ((state.rgbButtons[i] & 0x80) ? L'1' : L'0');
    }
    
    return oss.str();
}

std::wstring DisplayBuffer::FormatButtonState(const DIJOYSTATE2& state) {
    std::wstring result = L"Buttons: ";
    bool hasPressed = false;
    
    for (int i = 0; i < 32; ++i) {
        if (state.rgbButtons[i] & 0x80) {
            if (hasPressed) result += L", ";
            result += L"B" + std::to_wstring(i);
            hasPressed = true;
        }
    }
    
    if (!hasPressed) {
        result += L"None";
    }
    
    return result;
}
