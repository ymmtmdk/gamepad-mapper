#pragma once
#include <windows.h>
#include <dinput.h>
#include <vector>
#include <string>

// Logger interface for dependency injection
class ILogger {
public:
    virtual ~ILogger() = default;

    // Core logging methods
    virtual bool Init(const std::string& logFilePath) = 0;
    virtual void Close() = 0;

    virtual void Write(const char* fmt, ...) = 0;
    virtual void WriteW(const wchar_t* fmt, ...) = 0;

    // DEPRECATED: Frame logging methods - Use DisplayBuffer for screen display instead
    [[deprecated("Use DisplayBuffer for screen display functionality instead of frame log")]]
    virtual void ClearFrameLog() = 0;
    [[deprecated("Use DisplayBuffer for screen display functionality instead of frame log")]]
    virtual void AppendFrameLog(const wchar_t* fmt, ...) = 0;
    [[deprecated("Use DisplayBuffer for screen display functionality instead of frame log")]]
    virtual void AppendGamepadInfo(bool connected, const wchar_t* productName, const wchar_t* instanceName) = 0;
    [[deprecated("Use DisplayBuffer for screen display functionality instead of frame log")]]
    virtual void AppendState(const DIJOYSTATE2& js) = 0;
    [[deprecated("Use DisplayBuffer for screen display functionality instead of frame log")]]
    virtual void AppendLog(const std::wstring& message) = 0;
            
    // DEPRECATED: Accessor for frame log data
    [[deprecated("Use DisplayBuffer for screen display functionality instead of frame log")]]
    virtual const std::vector<std::wstring>& GetFrameLog() const = 0;
};// Forward declaration for dependency injection

class Logger;