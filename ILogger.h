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

    // Frame logging methods
    virtual void ClearFrameLog() = 0;
    virtual void AppendFrameLog(const wchar_t* fmt, ...) = 0;
    virtual void AppendGamepadInfo(bool connected, const wchar_t* productName, const wchar_t* instanceName) = 0;
    virtual void AppendState(const DIJOYSTATE2& js) = 0;
    virtual void AppendLog(const std::wstring& message) = 0;
            
    // Accessors
    virtual const std::vector<std::wstring>& GetFrameLog() const = 0;
};// Forward declaration for dependency injection

class Logger;