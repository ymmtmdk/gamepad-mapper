#include "Logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cwchar>
#include <codecvt>
#include <locale>

// Singleton instance
Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : m_isInitialized(false) {
    // Constructor now public for dependency injection
}

Logger::~Logger() {
    Close();
}

bool Logger::Init(const std::string& logFilePath) {
    try {
        // Create rotating file sink (5MB max size, 3 backup files)
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath, 1024 * 1024 * 5, 3);
        
        // Create console sink for debug builds
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        // Combine sinks
        std::vector<spdlog::sink_ptr> sinks{rotating_sink};
        
#ifdef _DEBUG
        sinks.push_back(console_sink);
#endif

        // Create logger with multiple sinks
        m_logger = std::make_shared<spdlog::logger>("gamepad_mapper", sinks.begin(), sinks.end());
        
        // Set pattern: [timestamp] [level] message
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        
        // Set log level (INFO for release, DEBUG for debug builds)
#ifdef _DEBUG
        m_logger->set_level(spdlog::level::debug);
#else
        m_logger->set_level(spdlog::level::info);
#endif

        // Register logger globally
        spdlog::register_logger(m_logger);
        
        // Enable automatic flushing on error level
        m_logger->flush_on(spdlog::level::err);
        
        m_isInitialized = true;
        
        // Log startup message
        m_logger->info("=== Gamepad to Keyboard Mapper Log Started ===");
        
        SYSTEMTIME st;
        GetLocalTime(&st);
        m_logger->info("Start Time: {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                       st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        
        return true;
    }
    catch (const spdlog::spdlog_ex& ex) {
        // Fallback error handling
        OutputDebugStringA("Logger initialization failed: ");
        OutputDebugStringA(ex.what());
        return false;
    }
}

void Logger::Close() {
    if (m_logger && m_isInitialized) {
        m_logger->info("=== Log Ended ===");
        m_logger->flush();
        spdlog::drop("gamepad_mapper");
        m_logger.reset();
        m_isInitialized = false;
    }
}

void Logger::Write(const char* fmt, ...) {
    if (!m_logger) return;
    
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    
    m_logger->info(buf);
}

void Logger::WriteW(const wchar_t* fmt, ...) {
    if (!m_logger) return;
    
    wchar_t buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vswprintf_s(buf, _countof(buf), fmt, ap);
    va_end(ap);
    
    std::string utf8_str = WStringToString(buf);
    m_logger->info(utf8_str);
}


// Wide string logging methods
void Logger::InfoW(const std::wstring& message) {
    if (m_logger) {
        try {
            std::string utf8_str = WStringToString(message);
            m_logger->info(utf8_str);
        }
        catch (const std::exception& e) {
            m_logger->error("Failed to convert wide string for logging: {}", e.what());
        }
    }
}

void Logger::DebugW(const std::wstring& message) {
    if (m_logger) {
        try {
            std::string utf8_str = WStringToString(message);
            m_logger->debug(utf8_str);
        }
        catch (const std::exception& e) {
            m_logger->error("Failed to convert wide string for logging: {}", e.what());
        }
    }
}

void Logger::WarnW(const std::wstring& message) {
    if (m_logger) {
        try {
            std::string utf8_str = WStringToString(message);
            m_logger->warn(utf8_str);
        }
        catch (const std::exception& e) {
            m_logger->error("Failed to convert wide string for logging: {}", e.what());
        }
    }
}

void Logger::ErrorW(const std::wstring& message) {
    if (m_logger) {
        try {
            std::string utf8_str = WStringToString(message);
            m_logger->error(utf8_str);
        }
        catch (const std::exception& e) {
            // Last resort error output
            OutputDebugStringW(L"Logger: Failed to log error message\n");
            OutputDebugStringW(message.c_str());
        }
    }
}

// Configuration methods
void Logger::SetLogLevel(spdlog::level::level_enum level) {
    if (m_logger) {
        m_logger->set_level(level);
    }
}

void Logger::EnableConsoleOutput(bool enable) {
    // This would require recreating the logger with/without console sink
    // For now, we'll just log the configuration change
    if (m_logger) {
        m_logger->info("Console output setting changed to: {}", enable ? "enabled" : "disabled");
    }
}

// Helper methods for string conversion
std::string Logger::WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring Logger::StringToWString(const std::string& str) {
    if (str.empty()) return {};
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}