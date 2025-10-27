#pragma once

/// Modern C++23 Core Infrastructure for GamepadMapper
/// This header provides the complete foundation for high-performance,
/// type-safe, and zero-cost abstraction based gamepad input processing.

// Error Handling & Monadic Operations
#include "Expected.h"

// Type System & Constraints  
#include "Concepts.h"

// Resource Management
#include "RAII.h"

// Type Erasure & Polymorphism
#include "TypeErasure.h"

// Algorithms & Functional Programming
#include "Algorithms.h"

// Memory Management & Performance
#include "Memory.h"

// Structured Logging
#include "Logging.h"

// Async Programming
#include "Coroutines.h"

namespace gm::core {

/// Version information
inline constexpr struct {
    int major = 2;
    int minor = 0;
    int patch = 0;
    const char* suffix = "modern";
} version;

/// Feature flags for compile-time configuration
template<bool UseCoroutines = true, 
         bool UseAsyncLogging = true,
         bool UseMemoryPools = true,
         bool UseTypeErasure = true>
struct FeatureConfig {
    static constexpr bool coroutines = UseCoroutines;
    static constexpr bool async_logging = UseAsyncLogging;
    static constexpr bool memory_pools = UseMemoryPools;
    static constexpr bool type_erasure = UseTypeErasure;
};

/// Default configuration for production builds
using ProductionConfig = FeatureConfig<true, true, true, true>;

/// Development configuration with extra debugging
using DevelopmentConfig = FeatureConfig<true, false, false, false>;

/// Configuration selection based on build type
#ifdef NDEBUG
    using DefaultConfig = ProductionConfig;
#else
    using DefaultConfig = DevelopmentConfig;
#endif

/// Global configuration access
template<typename Config = DefaultConfig>
constexpr auto get_config() -> Config {
    return Config{};
}

/// Utility to check if feature is enabled
template<typename Config = DefaultConfig>
constexpr bool has_coroutines() { return Config::coroutines; }

template<typename Config = DefaultConfig>
constexpr bool has_async_logging() { return Config::async_logging; }

template<typename Config = DefaultConfig>
constexpr bool has_memory_pools() { return Config::memory_pools; }

template<typename Config = DefaultConfig>
constexpr bool has_type_erasure() { return Config::type_erasure; }

/// Core initialization
class CoreSystem {
public:
    static auto initialize() -> VoidResult {
        LOG_INFO("Initializing GamepadMapper Core v{}.{}.{}-{}", 
                version.major, version.minor, version.patch, version.suffix);
        
        if constexpr (DefaultConfig::memory_pools) {
            // Initialize memory pools
            [[maybe_unused]] auto& mgr = MemoryManager::instance();
            LOG_DEBUG("Memory pools initialized");
        }
        
        if constexpr (DefaultConfig::async_logging) {
            // Initialize async logging
            LOG_DEBUG("Async logging system initialized");
        }
        
        LOG_INFO("Core system initialization complete");
        return {};
    }
    
    static void shutdown() {
        LOG_INFO("Shutting down GamepadMapper Core");
        
        if constexpr (DefaultConfig::async_logging) {
            // Flush async logs
            LOG_DEBUG("Flushing async logs");
        }
        
        LOG_INFO("Core system shutdown complete");
    }
};

/// RAII wrapper for core system
class CoreGuard {
public:
    CoreGuard() {
        if (auto result = CoreSystem::initialize(); !result) {
            throw std::runtime_error(std::format("Core initialization failed: {}", result.error().what()));
        }
    }
    
    ~CoreGuard() {
        CoreSystem::shutdown();
    }
    
    CoreGuard(const CoreGuard&) = delete;
    CoreGuard& operator=(const CoreGuard&) = delete;
    CoreGuard(CoreGuard&&) = delete;
    CoreGuard& operator=(CoreGuard&&) = delete;
};

} // namespace gm::core

/// Convenience macros for common operations
#define GM_CORE_INIT() ::gm::core::CoreGuard _core_guard

#define GM_EXPECT(expr) \
    ({ auto _result = (expr); \
       if (!_result) [[unlikely]] { \
           LOG_ERROR("Expectation failed: {} - {}", #expr, _result.error().what()); \
           return std::unexpected(_result.error()); \
       } \
       std::move(*_result); })

#define GM_TRY(expr) \
    ({ auto _result = (expr); \
       if (!_result) [[unlikely]] { \
           return std::unexpected(_result.error()); \
       } \
       std::move(*_result); })

#define GM_SCOPE_TIMER(name) \
    ::gm::core::logging::ScopedTimer _timer(name)

#define GM_FUNCTION_TIMER() \
    GM_SCOPE_TIMER(__FUNCTION__)

/// Performance hints
#define GM_LIKELY(x) [[likely]] (x)
#define GM_UNLIKELY(x) [[unlikely]] (x)
#define GM_ASSUME(x) [[assume(x)]]

/// Compiler attributes
#define GM_FORCEINLINE [[msvc::forceinline]]
#define GM_NOINLINE [[msvc::noinline]]
#define GM_COLD [[gnu::cold]]
#define GM_HOT [[gnu::hot]]