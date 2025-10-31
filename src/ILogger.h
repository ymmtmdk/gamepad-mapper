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

};// Forward declaration for dependency injection

class Logger;