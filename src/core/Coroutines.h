#pragma once
#include "Expected.h"
#include <coroutine>
#include <exception>
#include <future>
#include <chrono>

namespace gm::core::async {

/// Basic task coroutine for async operations
template<typename T = void>
class Task {
public:
    struct promise_type {
        auto get_return_object() -> Task {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        auto initial_suspend() noexcept -> std::suspend_always { return {}; }
        auto final_suspend() noexcept -> std::suspend_always { return {}; }
        
        void unhandled_exception() {
            m_exception = std::current_exception();
        }
        
        template<typename U>
        void return_value(U&& value) requires(!std::is_void_v<T>) {
            m_value = std::forward<U>(value);
        }
        
        void return_void() requires(std::is_void_v<T>) {}
        
        auto get_result() -> T requires(!std::is_void_v<T>) {
            if (m_exception) {
                std::rethrow_exception(m_exception);
            }
            return std::move(m_value);
        }
        
        void get_result() requires(std::is_void_v<T>) {
            if (m_exception) {
                std::rethrow_exception(m_exception);
            }
        }
        
    private:
        T m_value{} requires(!std::is_void_v<T>);
        std::exception_ptr m_exception;
    };
    
    using handle_type = std::coroutine_handle<promise_type>;
    
    explicit Task(handle_type handle) : m_handle(handle) {}
    
    ~Task() {
        if (m_handle) {
            m_handle.destroy();
        }
    }
    
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    
    Task(Task&& other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (m_handle) {
                m_handle.destroy();
            }
            m_handle = std::exchange(other.m_handle, {});
        }
        return *this;
    }
    
    auto resume() -> bool {
        if (!m_handle || m_handle.done()) {
            return false;
        }
        m_handle.resume();
        return !m_handle.done();
    }
    
    auto is_ready() const noexcept -> bool {
        return m_handle && m_handle.done();
    }
    
    auto get_result() -> T {
        if (!is_ready()) {
            throw std::runtime_error("Task not ready");
        }
        return m_handle.promise().get_result();
    }
    
    // Awaitable interface
    auto operator co_await() noexcept {
        struct Awaiter {
            Task& task;
            
            auto await_ready() noexcept -> bool {
                return task.is_ready();
            }
            
            auto await_suspend(std::coroutine_handle<> continuation) noexcept -> std::coroutine_handle<> {
                // Simple scheduler: resume immediately
                return task.m_handle;
            }
            
            auto await_resume() -> T {
                return task.get_result();
            }
        };
        
        return Awaiter{*this};
    }

private:
    handle_type m_handle;
};

/// Generator coroutine for lazy evaluation
template<typename T>
class Generator {
public:
    struct promise_type {
        auto get_return_object() -> Generator {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        auto initial_suspend() noexcept -> std::suspend_always { return {}; }
        auto final_suspend() noexcept -> std::suspend_always { return {}; }
        
        void unhandled_exception() {
            m_exception = std::current_exception();
        }
        
        void return_void() {}
        
        auto yield_value(T value) -> std::suspend_always {
            m_current_value = std::move(value);
            return {};
        }
        
        auto current_value() const -> const T& {
            return m_current_value;
        }
        
        void rethrow_if_exception() {
            if (m_exception) {
                std::rethrow_exception(m_exception);
            }
        }
        
    private:
        T m_current_value{};
        std::exception_ptr m_exception;
    };
    
    using handle_type = std::coroutine_handle<promise_type>;
    
    explicit Generator(handle_type handle) : m_handle(handle) {}
    
    ~Generator() {
        if (m_handle) {
            m_handle.destroy();
        }
    }
    
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    
    Generator(Generator&& other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (m_handle) {
                m_handle.destroy();
            }
            m_handle = std::exchange(other.m_handle, {});
        }
        return *this;
    }
    
    // Iterator interface
    class Iterator {
    public:
        explicit Iterator(handle_type handle) : m_handle(handle) {
            if (m_handle) {
                ++*this; // Start the generator
            }
        }
        
        auto operator++() -> Iterator& {
            m_handle.resume();
            if (m_handle.done()) {
                m_handle.promise().rethrow_if_exception();
                m_handle = {};
            }
            return *this;
        }
        
        auto operator*() const -> const T& {
            return m_handle.promise().current_value();
        }
        
        auto operator==(const Iterator& other) const noexcept -> bool {
            return m_handle == other.m_handle;
        }
        
    private:
        handle_type m_handle;
    };
    
    auto begin() -> Iterator {
        return Iterator{m_handle};
    }
    
    auto end() -> Iterator {
        return Iterator{nullptr};
    }

private:
    handle_type m_handle;
};

/// Async sleep utility
inline auto sleep_for(std::chrono::milliseconds duration) -> Task<> {
    // Simple implementation - in real scenario would integrate with event loop
    std::this_thread::sleep_for(duration);
    co_return;
}

/// When all coroutine - wait for multiple tasks
template<typename... Tasks>
auto when_all(Tasks&&... tasks) -> Task<std::tuple<typename Tasks::value_type...>> {
    // Simplified implementation - start all tasks
    (tasks.resume(), ...);
    
    // Wait for all to complete
    while (!(tasks.is_ready() && ...)) {
        std::this_thread::yield();
    }
    
    co_return std::make_tuple(tasks.get_result()...);
}

/// When any coroutine - wait for first completion
template<typename... Tasks>
auto when_any(Tasks&&... tasks) -> Task<std::variant<typename Tasks::value_type...>> {
    // Start all tasks
    (tasks.resume(), ...);
    
    // Wait for any to complete
    while (true) {
        std::size_t index = 0;
        auto check_task = [&index]<typename Task>(Task& task) -> bool {
            if (task.is_ready()) {
                return true;
            }
            ++index;
            return false;
        };
        
        if ((check_task(tasks) || ...)) {
            // One of the tasks completed
            break;
        }
        
        std::this_thread::yield();
    }
    
    // Return the first completed result (simplified)
    co_return std::get<0>(std::make_tuple(tasks.get_result()...));
}

/// Async result with timeout
template<typename T>
auto with_timeout(Task<T> task, std::chrono::milliseconds timeout) -> Task<Result<T>> {
    auto timeout_task = sleep_for(timeout);
    
    // Race between task completion and timeout
    // Simplified implementation
    auto start = std::chrono::steady_clock::now();
    
    while (!task.is_ready()) {
        if (std::chrono::steady_clock::now() - start > timeout) {
            co_return std::unexpected(make_error(ErrorCode::UnknownError, "Operation timed out"));
        }
        
        task.resume();
        std::this_thread::yield();
    }
    
    co_return task.get_result();
}

} // namespace gm::core::async