#pragma once
#include "Concepts.h"
#include <memory>
#include <memory_resource>
#include <vector>
#include <atomic>
#include <new>

namespace gm::core {

/// High-performance memory management for game input processing
namespace memory {

/// Stack allocator for temporary allocations (no heap allocation)
template<std::size_t Size>
class StackAllocator {
public:
    StackAllocator() : m_top(m_buffer) {}
    
    template<typename T, typename... Args>
    auto allocate(Args&&... args) -> T* {
        static_assert(alignof(T) <= alignof(std::max_align_t));
        
        auto* aligned_ptr = std::align(alignof(T), sizeof(T), m_top, remaining_size());
        if (!aligned_ptr) {
            throw std::bad_alloc{};
        }
        
        auto* result = new(aligned_ptr) T(std::forward<Args>(args)...);
        m_top = static_cast<std::byte*>(aligned_ptr) + sizeof(T);
        return result;
    }
    
    void reset() noexcept {
        m_top = m_buffer;
    }
    
    [[nodiscard]] auto remaining_size() const noexcept -> std::size_t {
        return Size - (static_cast<std::byte*>(m_top) - m_buffer);
    }
    
    [[nodiscard]] auto is_empty() const noexcept -> bool {
        return m_top == m_buffer;
    }

private:
    alignas(std::max_align_t) std::byte m_buffer[Size];
    void* m_top;
};

/// Object pool for frequent allocations/deallocations
template<Poolable T>
class ObjectPool {
public:
    explicit ObjectPool(std::size_t initial_size = T::pool_size) {
        m_pool.reserve(initial_size);
        for (std::size_t i = 0; i < initial_size; ++i) {
            m_pool.emplace_back(std::make_unique<T>());
        }
    }
    
    auto acquire() -> std::unique_ptr<T> {
        if (!m_pool.empty()) {
            auto obj = std::move(m_pool.back());
            m_pool.pop_back();
            return obj;
        }
        
        return std::make_unique<T>();
    }
    
    void release(std::unique_ptr<T> obj) {
        if (obj && m_pool.size() < T::pool_size * 2) {
            // Reset object state if needed
            if constexpr (requires { obj->reset(); }) {
                obj->reset();
            }
            m_pool.emplace_back(std::move(obj));
        }
    }
    
    [[nodiscard]] auto available_count() const noexcept -> std::size_t {
        return m_pool.size();
    }

private:
    std::vector<std::unique_ptr<T>> m_pool;
};

/// Lock-free object pool for high-performance scenarios
template<Poolable T>
class LockFreeObjectPool {
public:
    explicit LockFreeObjectPool(std::size_t size = T::pool_size) : m_size(size) {
        m_storage = std::make_unique<Storage[]>(size);
        
        // Initialize free list
        for (std::size_t i = 0; i < size - 1; ++i) {
            m_storage[i].next.store(i + 1, std::memory_order_relaxed);
        }
        m_storage[size - 1].next.store(INVALID_INDEX, std::memory_order_relaxed);
        m_head.store(0, std::memory_order_relaxed);
    }
    
    auto acquire() -> T* {
        auto current_head = m_head.load(std::memory_order_acquire);
        
        while (current_head != INVALID_INDEX) {
            auto& storage = m_storage[current_head];
            auto next = storage.next.load(std::memory_order_relaxed);
            
            if (m_head.compare_exchange_weak(current_head, next, std::memory_order_release, std::memory_order_acquire)) {
                return new(&storage.data) T{};
            }
        }
        
        return nullptr; // Pool exhausted
    }
    
    void release(T* obj) noexcept {
        if (!obj) return;
        
        obj->~T();
        
        auto index = static_cast<std::size_t>(reinterpret_cast<Storage*>(obj) - m_storage.get());
        auto& storage = m_storage[index];
        
        auto current_head = m_head.load(std::memory_order_relaxed);
        do {
            storage.next.store(current_head, std::memory_order_relaxed);
        } while (!m_head.compare_exchange_weak(current_head, index, std::memory_order_release, std::memory_order_relaxed));
    }

private:
    static constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();
    
    struct Storage {
        alignas(T) std::byte data[sizeof(T)];
        std::atomic<std::size_t> next{INVALID_INDEX};
    };
    
    std::unique_ptr<Storage[]> m_storage;
    std::atomic<std::size_t> m_head{INVALID_INDEX};
    std::size_t m_size;
};

/// RAII wrapper for pool-allocated objects
template<typename T, typename Pool>
class PooledObject {
public:
    PooledObject(Pool& pool, T* obj) : m_pool(&pool), m_obj(obj) {}
    
    ~PooledObject() {
        if (m_obj) {
            m_pool->release(m_obj);
        }
    }
    
    PooledObject(const PooledObject&) = delete;
    PooledObject& operator=(const PooledObject&) = delete;
    
    PooledObject(PooledObject&& other) noexcept 
        : m_pool(std::exchange(other.m_pool, nullptr))
        , m_obj(std::exchange(other.m_obj, nullptr)) {}
    
    PooledObject& operator=(PooledObject&& other) noexcept {
        if (this != &other) {
            if (m_obj) {
                m_pool->release(m_obj);
            }
            m_pool = std::exchange(other.m_pool, nullptr);
            m_obj = std::exchange(other.m_obj, nullptr);
        }
        return *this;
    }
    
    auto get() const noexcept -> T* { return m_obj; }
    auto operator->() const noexcept -> T* { return m_obj; }
    auto operator*() const noexcept -> T& { return *m_obj; }
    
    explicit operator bool() const noexcept { return m_obj != nullptr; }

private:
    Pool* m_pool;
    T* m_obj;
};

/// Custom allocator using std::pmr for specific memory resources
template<typename T>
class PmrAllocator {
public:
    using value_type = T;
    
    explicit PmrAllocator(std::pmr::memory_resource* resource = std::pmr::get_default_resource()) noexcept
        : m_resource(resource) {}
    
    template<typename U>
    PmrAllocator(const PmrAllocator<U>& other) noexcept : m_resource(other.resource()) {}
    
    auto allocate(std::size_t n) -> T* {
        return static_cast<T*>(m_resource->allocate(n * sizeof(T), alignof(T)));
    }
    
    void deallocate(T* p, std::size_t n) noexcept {
        m_resource->deallocate(p, n * sizeof(T), alignof(T));
    }
    
    auto resource() const noexcept -> std::pmr::memory_resource* {
        return m_resource;
    }
    
    template<typename U>
    auto operator==(const PmrAllocator<U>& other) const noexcept -> bool {
        return m_resource == other.resource();
    }

private:
    std::pmr::memory_resource* m_resource;
};

/// Memory-mapped buffer for large sequential data
class MemoryMappedBuffer {
public:
    explicit MemoryMappedBuffer(std::size_t size);
    ~MemoryMappedBuffer();
    
    MemoryMappedBuffer(const MemoryMappedBuffer&) = delete;
    MemoryMappedBuffer& operator=(const MemoryMappedBuffer&) = delete;
    
    MemoryMappedBuffer(MemoryMappedBuffer&& other) noexcept;
    MemoryMappedBuffer& operator=(MemoryMappedBuffer&& other) noexcept;
    
    auto data() noexcept -> std::byte* { return m_data; }
    auto data() const noexcept -> const std::byte* { return m_data; }
    auto size() const noexcept -> std::size_t { return m_size; }
    
    template<typename T>
    auto as() noexcept -> T* {
        static_assert(std::is_trivially_copyable_v<T>);
        return reinterpret_cast<T*>(m_data);
    }
    
    template<typename T>
    auto as() const noexcept -> const T* {
        static_assert(std::is_trivially_copyable_v<T>);
        return reinterpret_cast<const T*>(m_data);
    }

private:
    std::byte* m_data = nullptr;
    std::size_t m_size = 0;
    void* m_handle = nullptr; // HANDLE on Windows
};

/// Circular buffer for real-time data streaming
template<typename T, std::size_t Capacity>
class CircularBuffer {
public:
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    
    auto push(const T& item) noexcept -> bool {
        auto current_tail = m_tail.load(std::memory_order_relaxed);
        auto next_tail = (current_tail + 1) & (Capacity - 1);
        
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        
        m_buffer[current_tail] = item;
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }
    
    auto pop() noexcept -> std::optional<T> {
        auto current_head = m_head.load(std::memory_order_relaxed);
        
        if (current_head == m_tail.load(std::memory_order_acquire)) {
            return std::nullopt; // Buffer empty
        }
        
        auto item = m_buffer[current_head];
        m_head.store((current_head + 1) & (Capacity - 1), std::memory_order_release);
        return item;
    }
    
    [[nodiscard]] auto size() const noexcept -> std::size_t {
        auto tail = m_tail.load(std::memory_order_acquire);
        auto head = m_head.load(std::memory_order_acquire);
        return (tail - head) & (Capacity - 1);
    }
    
    [[nodiscard]] auto empty() const noexcept -> bool {
        return m_head.load(std::memory_order_acquire) == m_tail.load(std::memory_order_acquire);
    }
    
    [[nodiscard]] auto full() const noexcept -> bool {
        auto tail = m_tail.load(std::memory_order_acquire);
        auto head = m_head.load(std::memory_order_acquire);
        return ((tail + 1) & (Capacity - 1)) == head;
    }

private:
    alignas(64) std::array<T, Capacity> m_buffer; // Cache line aligned
    alignas(64) std::atomic<std::size_t> m_head{0};
    alignas(64) std::atomic<std::size_t> m_tail{0};
};

} // namespace memory

/// Global memory resource management
class MemoryManager {
public:
    static auto instance() -> MemoryManager& {
        static MemoryManager mgr;
        return mgr;
    }
    
    auto get_device_pool() -> memory::LockFreeObjectPool<class GamepadDevice>& {
        static memory::LockFreeObjectPool<GamepadDevice> pool;
        return pool;
    }
    
    auto get_temp_allocator() -> memory::StackAllocator<1024 * 1024>& { // 1MB stack
        thread_local memory::StackAllocator<1024 * 1024> allocator;
        return allocator;
    }
    
    auto get_pmr_resource() -> std::pmr::memory_resource* {
        if (!m_pmr_resource) {
            m_pmr_resource = std::pmr::new_delete_resource();
        }
        return m_pmr_resource;
    }

private:
    std::pmr::memory_resource* m_pmr_resource = nullptr;
};

} // namespace gm::core