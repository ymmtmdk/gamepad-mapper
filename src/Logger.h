#pragma once
#include "ILogger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

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
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Helper methods
    std::string WStringToString(const std::wstring& wstr);
    std::wstring StringToWString(const std::string& str);

    // Member variables
    std::shared_ptr<spdlog::logger> m_logger;
    bool m_isInitialized;
};

// New modern macros for improved logging
#define LOG_INFO(...) Logger::GetInstance().Info(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::GetInstance().Debug(__VA_ARGS__)
#define LOG_WARN(...) Logger::GetInstance().Warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::GetInstance().Error(__VA_ARGS__)

// Wide string macros for Windows
#define LOG_INFO_W(msg) Logger::GetInstance().InfoW(msg)
#define LOG_DEBUG_W(msg) Logger::GetInstance().DebugW(msg)
#define LOG_WARN_W(msg) Logger::GetInstance().WarnW(msg)
#define LOG_ERROR_W(msg) Logger::GetInstance().ErrorW(msg)


// Backward compatibility macros (DEPRECATED - for old Logger.h compatibility)
#define MODERN_LOG_WRITE(...) Logger::GetInstance().Write(__VA_ARGS__)
#define MODERN_LOG_WRITE_W(...) Logger::GetInstance().WriteW(__VA_ARGS__)

// Log level configuration macros
#define LOG_SET_LEVEL(level) Logger::GetInstance().SetLogLevel(level)
#define LOG_ENABLE_CONSOLE(enable) Logger::GetInstance().EnableConsoleOutput(enable)