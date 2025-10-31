#pragma once
#include "ILogger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

class ModernLogger : public ILogger {
public:
    // Constructor/Destructor for dependency injection
    ModernLogger();
    ~ModernLogger() override;

    // Singleton access (backward compatibility - will be deprecated)
    static ModernLogger& GetInstance();

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

    // Modern logging methods with levels
    template<typename... Args>
    void Info(const std::string& fmt, Args&&... args) {
        if (m_logger) {
            m_logger->info(fmt::runtime(fmt), std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void Debug(const std::string& fmt, Args&&... args) {
        if (m_logger) {
            m_logger->debug(fmt::runtime(fmt), std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void Warn(const std::string& fmt, Args&&... args) {
        if (m_logger) {
            m_logger->warn(fmt::runtime(fmt), std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void Error(const std::string& fmt, Args&&... args) {
        if (m_logger) {
            m_logger->error(fmt::runtime(fmt), std::forward<Args>(args)...);
        }
    }

    // Wide string versions for Windows compatibility
    void InfoW(const std::wstring& message);
    void DebugW(const std::wstring& message);
    void WarnW(const std::wstring& message);
    void ErrorW(const std::wstring& message);

    // Configuration methods
    void SetLogLevel(spdlog::level::level_enum level);
    void EnableConsoleOutput(bool enable);

private:
    // Copy operations disabled for singleton compatibility
    ModernLogger(const ModernLogger&) = delete;
    ModernLogger& operator=(const ModernLogger&) = delete;

    // Helper methods
    std::string WStringToString(const std::wstring& wstr);
    std::wstring StringToWString(const std::string& str);

    // Member variables
    std::shared_ptr<spdlog::logger> m_logger;
    std::vector<std::wstring> m_frameLog;
    mutable std::mutex m_frameLogMutex;
    bool m_isInitialized;
};

// New modern macros for improved logging
#define LOG_INFO(...) ModernLogger::GetInstance().Info(__VA_ARGS__)
#define LOG_DEBUG(...) ModernLogger::GetInstance().Debug(__VA_ARGS__)
#define LOG_WARN(...) ModernLogger::GetInstance().Warn(__VA_ARGS__)
#define LOG_ERROR(...) ModernLogger::GetInstance().Error(__VA_ARGS__)

// Wide string macros for Windows
#define LOG_INFO_W(msg) ModernLogger::GetInstance().InfoW(msg)
#define LOG_DEBUG_W(msg) ModernLogger::GetInstance().DebugW(msg)
#define LOG_WARN_W(msg) ModernLogger::GetInstance().WarnW(msg)
#define LOG_ERROR_W(msg) ModernLogger::GetInstance().ErrorW(msg)

// Frame logging convenience macros
#define FRAME_LOG_CLEAR() ModernLogger::GetInstance().ClearFrameLog()
#define FRAME_LOG_APPEND(...) ModernLogger::GetInstance().AppendFrameLog(__VA_ARGS__)
#define FRAME_LOG_GAMEPAD_INFO(connected, product, instance) ModernLogger::GetInstance().AppendGamepadInfo(connected, product, instance)
#define FRAME_LOG_STATE(js) ModernLogger::GetInstance().AppendState(js)

// Backward compatibility macros (DEPRECATED - for old Logger.h compatibility)
#define MODERN_LOG_WRITE(...) ModernLogger::GetInstance().Write(__VA_ARGS__)
#define MODERN_LOG_WRITE_W(...) ModernLogger::GetInstance().WriteW(__VA_ARGS__)
#define MODERN_FRAME_LOG_APPEND(...) ModernLogger::GetInstance().AppendFrameLog(__VA_ARGS__)

// Log level configuration macros
#define LOG_SET_LEVEL(level) ModernLogger::GetInstance().SetLogLevel(level)
#define LOG_ENABLE_CONSOLE(enable) ModernLogger::GetInstance().EnableConsoleOutput(enable)