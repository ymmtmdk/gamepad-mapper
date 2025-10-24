#pragma once
#include "ILogger.h"
#include <windows.h>
#include <dinput.h>
#include <vector>
#include <string>
#include <fstream>
#include <memory>

class Logger : public ILogger {
public:
    // Constructor/Destructor for dependency injection
    Logger();
    ~Logger() override;

    // Singleton access (backward compatibility - will be deprecated)
    static Logger& GetInstance();

    // ILogger interface implementation
    bool Init(const std::string& logFilePath) override;
    void Close() override;

    void Write(const char* fmt, ...) override;
    void WriteW(const wchar_t* fmt, ...) override;

    void ClearFrameLog() override;
    void AppendFrameLog(const wchar_t* fmt, ...) override;
    void AppendGamepadInfo(bool connected, const wchar_t* productName, const wchar_t* instanceName) override;
    void AppendState(const DIJOYSTATE2& js) override;
    void AppendLog(const std::wstring& message) override;

    // Accessors
    const std::vector<std::wstring>& GetFrameLog() const override;

private:
    // Copy operations disabled for singleton compatibility
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void WriteTimestamp();

    std::wofstream m_logFile;
    std::vector<std::wstring> m_frameLog;
};

// Helper macro for easy access
#define LOG_WRITE(...) Logger::GetInstance().Write(__VA_ARGS__)
#define LOG_WRITE_W(...) Logger::GetInstance().WriteW(__VA_ARGS__)
#define FRAME_LOG_APPEND(...) Logger::GetInstance().AppendFrameLog(__VA_ARGS__)
