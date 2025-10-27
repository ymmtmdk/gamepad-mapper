#pragma once
#include "Expected.h"
#include "Concepts.h"
#include <source_location>
#include <format>
#include <chrono>
#include <string_view>
#include <atomic>
#include <array>

namespace gm::core::logging {

/// Compile-time log levels
enum class Level : std::uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Critical = 5,
    Off = 6
};

/// Compile-time log level configuration
template<Level MinLevel = Level::Info>
struct LogConfig {
    static constexpr Level min_level = MinLevel;
    static constexpr bool async_logging = true;
    static constexpr std::size_t buffer_size = 8192;
};

/// Zero-cost logging with compile-time elimination
template<LogConfig Config = LogConfig<>>
class Logger {
public:
    template<Level LogLevel, typename... Args>
    static constexpr void log(std::string_view format, 
                             Args&&... args,
                             std::source_location location = std::source_location::current()) {
        if constexpr (LogLevel >= Config.min_level) {
            log_impl<LogLevel>(format, std::forward<Args>(args)..., location);
        }
    }
    
    template<typename... Args>
    static constexpr void trace(std::string_view format, Args&&... args,
                               std::source_location location = std::source_location::current()) {
        log<Level::Trace>(format, std::forward<Args>(args)..., location);
    }
    
    template<typename... Args>
    static constexpr void debug(std::string_view format, Args&&... args,
                               std::source_location location = std::source_location::current()) {
        log<Level::Debug>(format, std::forward<Args>(args)..., location);
    }
    
    template<typename... Args>
    static constexpr void info(std::string_view format, Args&&... args,
                              std::source_location location = std::source_location::current()) {
        log<Level::Info>(format, std::forward<Args>(args)..., location);
    }
    
    template<typename... Args>
    static constexpr void warn(std::string_view format, Args&&... args,
                              std::source_location location = std::source_location::current()) {
        log<Level::Warn>(format, std::forward<Args>(args)..., location);
    }
    
    template<typename... Args>
    static constexpr void error(std::string_view format, Args&&... args,
                               std::source_location location = std::source_location::current()) {
        log<Level::Error>(format, std::forward<Args>(args)..., location);
    }
    
    template<typename... Args>
    static constexpr void critical(std::string_view format, Args&&... args,
                                  std::source_location location = std::source_location::current()) {
        log<Level::Critical>(format, std::forward<Args>(args)..., location);
    }

private:
    template<Level LogLevel, typename... Args>
    static void log_impl(std::string_view format, Args&&... args, std::source_location location) {
        auto message = std::format(format, std::forward<Args>(args)...);
        auto timestamp = std::chrono::system_clock::now();
        
        LogEntry entry{
            .level = LogLevel,
            .message = std::move(message),
            .location = location,
            .timestamp = timestamp
        };
        
        if constexpr (Config.async_logging) {
            get_async_sink().write(std::move(entry));
        } else {
            get_sync_sink().write(entry);
        }
    }
    
    struct LogEntry {
        Level level;
        std::string message;
        std::source_location location;
        std::chrono::system_clock::time_point timestamp;
    };
    
    static auto get_async_sink() -> auto&;
    static auto get_sync_sink() -> auto&;
};

/// Structured logging with compile-time field validation
template<typename... Fields>
struct StructuredLog {
    std::tuple<Fields...> fields;
    
    template<typename T>
    constexpr auto get() const -> const T& {
        static_assert((std::is_same_v<T, Fields> || ...), "Field type not found in log structure");
        return std::get<T>(fields);
    }
    
    constexpr auto format() const -> std::string {
        return std::apply([](const auto&... fields) {
            return std::format("{}", (fields.format() + ...));
        }, fields);
    }
};

/// Predefined structured log fields
struct DeviceField {
    std::string_view name;
    constexpr auto format() const -> std::string { return std::format("device={} ", name); }
};

struct ErrorField {
    std::uint32_t code;
    constexpr auto format() const -> std::string { return std::format("error_code=0x{:08X} ", code); }
};

struct DurationField {
    std::chrono::microseconds duration;
    constexpr auto format() const -> std::string { return std::format("duration={}μs ", duration.count()); }
};

struct CountField {
    std::size_t count;
    constexpr auto format() const -> std::string { return std::format("count={} ", count); }
};

/// Type-safe log macros with zero runtime cost when disabled
#ifdef NDEBUG
    using ProductionLogger = Logger<LogConfig<Level::Info>>;
    #define LOG_TRACE(...) do {} while(0)
    #define LOG_DEBUG(...) do {} while(0)
#else
    using ProductionLogger = Logger<LogConfig<Level::Trace>>;
    #define LOG_TRACE(...) ::gm::core::logging::ProductionLogger::trace(__VA_ARGS__)
    #define LOG_DEBUG(...) ::gm::core::logging::ProductionLogger::debug(__VA_ARGS__)
#endif

#define LOG_INFO(...) ::gm::core::logging::ProductionLogger::info(__VA_ARGS__)
#define LOG_WARN(...) ::gm::core::logging::ProductionLogger::warn(__VA_ARGS__)
#define LOG_ERROR(...) ::gm::core::logging::ProductionLogger::error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::gm::core::logging::ProductionLogger::critical(__VA_ARGS__)

/// Structured logging macros
#define LOG_STRUCTURED(level, ...) \
    ::gm::core::logging::ProductionLogger::log<level>("{}", \
        ::gm::core::logging::StructuredLog{std::make_tuple(__VA_ARGS__)}.format())

#define LOG_DEVICE_EVENT(level, device_name, ...) \
    LOG_STRUCTURED(level, ::gm::core::logging::DeviceField{device_name}, __VA_ARGS__)

#define LOG_ERROR_CODE(error_code, ...) \
    LOG_STRUCTURED(::gm::core::logging::Level::Error, \
        ::gm::core::logging::ErrorField{error_code}, __VA_ARGS__)

/// Performance logging with automatic duration measurement
class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view name) 
        : m_name(name), m_start(std::chrono::high_resolution_clock::now()) {}
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
        LOG_STRUCTURED(Level::Debug, 
            DurationField{duration},
            DeviceField{m_name});
    }

private:
    std::string_view m_name;
    std::chrono::high_resolution_clock::time_point m_start;
};

#define LOG_SCOPE_TIMER(name) ::gm::core::logging::ScopedTimer _timer(name)
#define LOG_FUNCTION_TIMER() LOG_SCOPE_TIMER(__FUNCTION__)

/// Lock-free ring buffer for async logging
template<std::size_t Size>
class AsyncLogSink {
public:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
    
    template<typename LogEntry>
    auto write(LogEntry&& entry) -> bool {
        auto current_tail = m_tail.load(std::memory_order_relaxed);
        auto next_tail = (current_tail + 1) & (Size - 1);
        
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        
        m_buffer[current_tail] = std::forward<LogEntry>(entry);
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }
    
    template<typename F>
    auto consume_all(F&& consumer) -> std::size_t {
        std::size_t count = 0;
        
        while (auto entry = read_one()) {
            consumer(*entry);
            ++count;
        }
        
        return count;
    }

private:
    auto read_one() -> std::optional<LogEntry> {
        auto current_head = m_head.load(std::memory_order_relaxed);
        
        if (current_head == m_tail.load(std::memory_order_acquire)) {
            return std::nullopt; // Buffer empty
        }
        
        auto entry = std::move(m_buffer[current_head]);
        m_head.store((current_head + 1) & (Size - 1), std::memory_order_release);
        return entry;
    }
    
    struct LogEntry {
        Level level;
        std::string message;
        std::source_location location;
        std::chrono::system_clock::time_point timestamp;
    };
    
    alignas(64) std::array<LogEntry, Size> m_buffer;
    alignas(64) std::atomic<std::size_t> m_head{0};
    alignas(64) std::atomic<std::size_t> m_tail{0};
};

} // namespace gm::core::logging