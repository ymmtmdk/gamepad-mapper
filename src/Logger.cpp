#include "Logger.h"
#include <cstdio>
#include <cstdarg>
#include <cwchar>

// Singleton instance
Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    // Constructor now public for dependency injection
}

Logger::~Logger() {
    Close();
}

bool Logger::Init(const std::string& logFilePath) {
    m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!m_logFile.is_open()) {
        return false;
    }

    SYSTEMTIME st;
    GetLocalTime(&st);
    WriteW(L"=== Gamepad to Keyboard Mapper Log Started ===");
    WriteW(L"Start Time: %04d-%02d-%02d %02d:%02d:%02d",
           st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return true;
}

void Logger::AppendLog(const std::wstring& message)
{
    m_frameLog.push_back(message);
}

const std::vector<std::wstring>& Logger::GetFrameLog() const {
    return m_frameLog;
}

void Logger::Close()
{
    if (m_logFile.is_open()) {
        WriteW(L"=== Log Ended ===");
        m_logFile.close();
    }
}

void Logger::WriteTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timestamp[64];
    swprintf_s(timestamp, L"[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    m_logFile << timestamp;
}

void Logger::Write(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    // Convert to wide string to write to wofstream
    wchar_t wbuf[1024];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wbuf, buf, _TRUNCATE);

    WriteTimestamp();
    m_logFile << wbuf << std::endl;
	m_logFile.flush();
}

void Logger::WriteW(const wchar_t* fmt, ...) {
    wchar_t buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vswprintf_s(buf, _countof(buf), fmt, ap);
    va_end(ap);

    WriteTimestamp();
    m_logFile << buf << std::endl;
    m_logFile.flush();
}

void Logger::ClearFrameLog() {
    m_frameLog.clear();
}

void Logger::AppendFrameLog(const wchar_t* fmt, ...) {
    wchar_t buf[512];
    va_list ap;
    va_start(ap, fmt);
    vswprintf_s(buf, _countof(buf), fmt, ap);
    va_end(ap);
    m_frameLog.emplace_back(buf);
}

void Logger::AppendGamepadInfo(bool connected, const wchar_t* productName, const wchar_t* instanceName) {
    if (connected) {
        AppendFrameLog(L"=== gamepad ===");
        AppendFrameLog(L"name: %s", productName);
        AppendFrameLog(L"instance name: %s", instanceName);
        AppendFrameLog(L"status: connected");
        AppendFrameLog(L""); // Empty line
    } else {
        AppendFrameLog(L"=== gamepad ===");
        AppendFrameLog(L"statuc: not connected");
        AppendFrameLog(L""); // Empty line
    }
}

void Logger::AppendState(const DIJOYSTATE2& js) {
    AppendFrameLog(L"Axes: X=%ld Y=%ld Z=%ld RX=%ld RY=%ld RZ=%ld", js.lX, js.lY, js.lZ, js.lRx, js.lRy, js.lRz);
    AppendFrameLog(L"Sliders: S0=%ld S1=%ld", js.rglSlider[0], js.rglSlider[1]);

    for (int i = 0; i < 4; ++i) {
        DWORD pov = js.rgdwPOV[i];
        if (pov == 0xFFFFFFFF) AppendFrameLog(L"POV%d: -", i);
        else AppendFrameLog(L"POV%d: %lu", i, pov);
    }

    std::wstring s = L"Btns:";
    for (int i = 0; i < 32; ++i) {
        s.push_back((js.rgbButtons[i] & 0x80) ? L'1' : L'0');
        if ((i + 1) % 8 == 0 && i + 1 < 32) s.push_back(L' ');
    }
    m_frameLog.push_back(s);
}
