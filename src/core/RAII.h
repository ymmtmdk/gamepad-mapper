#pragma once
#include "Expected.h"
#include "Concepts.h"
#include <memory>
#include <vector>
#include <functional>
#include <utility>

namespace gm::core {

/// RAII++ Pattern: Automatic rollback on failure
class RaiiChain {
public:
    using CleanupFunction = std::function<void()>;
    using StepFunction = std::function<VoidResult()>;

    RaiiChain() = default;
    ~RaiiChain() { cleanup(); }

    // Non-copyable, movable
    RaiiChain(const RaiiChain&) = delete;
    RaiiChain& operator=(const RaiiChain&) = delete;
    RaiiChain(RaiiChain&&) = default;
    RaiiChain& operator=(RaiiChain&&) = default;

    /// Add a step with automatic cleanup registration
    template<typename F, typename C>
    auto add(F&& step, C&& cleanup) -> RaiiChain& {
        if (auto result = step(); !result) {
            m_error = std::move(result.error());
            return *this;
        }
        m_cleanups.emplace_back(std::forward<C>(cleanup));
        return *this;
    }

    /// Add a step that returns both value and cleanup
    template<typename F>
        requires std::invocable<F> && 
                 std::same_as<std::invoke_result_t<F>, 
                             Result<std::pair<void*, CleanupFunction>>>
    auto add(F&& step) -> RaiiChain& {
        if (auto result = step(); result) {
            auto [_, cleanup] = std::move(*result);
            m_cleanups.emplace_back(std::move(cleanup));
        } else {
            m_error = std::move(result.error());
        }
        return *this;
    }

    /// Finalize the chain - commit or rollback
    auto finalize() -> VoidResult {
        if (m_error) {
            cleanup();
            return std::unexpected(std::move(*m_error));
        }
        
        // Success - release ownership of cleanups
        m_cleanups.clear();
        return {};
    }

    /// Check if chain is still valid
    auto is_valid() const noexcept -> bool {
        return !m_error.has_value();
    }

private:
    void cleanup() {
        // Execute cleanups in reverse order
        for (auto it = m_cleanups.rbegin(); it != m_cleanups.rend(); ++it) {
            try {
                (*it)();
            } catch (...) {
                // Cleanup should never throw, but be safe
            }
        }
        m_cleanups.clear();
    }

    std::vector<CleanupFunction> m_cleanups;
    std::optional<Error> m_error;
};

/// RAII wrapper for Windows handles
template<typename HandleType, auto InvalidValue, auto DeleterFunc>
class UniqueHandle {
public:
    using handle_type = HandleType;
    
    constexpr UniqueHandle() noexcept : m_handle(InvalidValue) {}
    
    explicit UniqueHandle(HandleType handle) noexcept : m_handle(handle) {}
    
    ~UniqueHandle() { reset(); }
    
    // Non-copyable, movable
    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;
    
    UniqueHandle(UniqueHandle&& other) noexcept : m_handle(std::exchange(other.m_handle, InvalidValue)) {}
    
    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            reset();
            m_handle = std::exchange(other.m_handle, InvalidValue);
        }
        return *this;
    }
    
    void reset(HandleType new_handle = InvalidValue) noexcept {
        if (m_handle != InvalidValue) {
            DeleterFunc(m_handle);
        }
        m_handle = new_handle;
    }
    
    [[nodiscard]] auto release() noexcept -> HandleType {
        return std::exchange(m_handle, InvalidValue);
    }
    
    [[nodiscard]] auto get() const noexcept -> HandleType {
        return m_handle;
    }
    
    [[nodiscard]] auto get_address_of() noexcept -> HandleType* {
        reset();
        return &m_handle;
    }
    
    [[nodiscard]] explicit operator bool() const noexcept {
        return m_handle != InvalidValue;
    }
    
    [[nodiscard]] auto operator*() const noexcept -> HandleType {
        return m_handle;
    }

private:
    HandleType m_handle;
};

/// Common Windows handle types
using UniqueFileHandle = UniqueHandle<HANDLE, INVALID_HANDLE_VALUE, &CloseHandle>;
using UniqueLibraryHandle = UniqueHandle<HMODULE, nullptr, &FreeLibrary>;

/// RAII wrapper for COM objects
template<typename T>
class ComPtr {
public:
    using element_type = T;
    
    constexpr ComPtr() noexcept = default;
    
    explicit ComPtr(T* ptr) noexcept : m_ptr(ptr) {
        if (m_ptr) m_ptr->AddRef();
    }
    
    ~ComPtr() { reset(); }
    
    // Copyable and movable
    ComPtr(const ComPtr& other) noexcept : m_ptr(other.m_ptr) {
        if (m_ptr) m_ptr->AddRef();
    }
    
    ComPtr& operator=(const ComPtr& other) noexcept {
        if (this != &other) {
            reset();
            m_ptr = other.m_ptr;
            if (m_ptr) m_ptr->AddRef();
        }
        return *this;
    }
    
    ComPtr(ComPtr&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)) {}
    
    ComPtr& operator=(ComPtr&& other) noexcept {
        if (this != &other) {
            reset();
            m_ptr = std::exchange(other.m_ptr, nullptr);
        }
        return *this;
    }
    
    void reset(T* ptr = nullptr) noexcept {
        if (m_ptr) {
            m_ptr->Release();
        }
        m_ptr = ptr;
        if (m_ptr) {
            m_ptr->AddRef();
        }
    }
    
    [[nodiscard]] auto get() const noexcept -> T* {
        return m_ptr;
    }
    
    [[nodiscard]] auto get_address_of() noexcept -> T** {
        reset();
        return &m_ptr;
    }
    
    [[nodiscard]] auto release() noexcept -> T* {
        return std::exchange(m_ptr, nullptr);
    }
    
    [[nodiscard]] explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }
    
    [[nodiscard]] auto operator->() const noexcept -> T* {
        return m_ptr;
    }
    
    [[nodiscard]] auto operator*() const noexcept -> T& {
        return *m_ptr;
    }

private:
    T* m_ptr = nullptr;
};

/// Scoped action executor
template<Invocable<> F>
class ScopeGuard {
public:
    explicit ScopeGuard(F&& func) : m_func(std::forward<F>(func)), m_active(true) {}
    
    ~ScopeGuard() {
        if (m_active) {
            m_func();
        }
    }
    
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    
    ScopeGuard(ScopeGuard&& other) noexcept 
        : m_func(std::move(other.m_func)), m_active(std::exchange(other.m_active, false)) {}
    
    void dismiss() noexcept {
        m_active = false;
    }

private:
    F m_func;
    bool m_active;
};

template<typename F>
ScopeGuard(F&&) -> ScopeGuard<F>;

/// Convenience macro for scope guards
#define SCOPE_EXIT(...) auto CONCAT(_scope_guard_, __LINE__) = ::gm::core::ScopeGuard([&]() { __VA_ARGS__; })
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define CONCAT_IMPL(a, b) a##b

} // namespace gm::core