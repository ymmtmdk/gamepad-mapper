#pragma once
#include "Expected.h"
#include "Concepts.h"
#include <memory>
#include <typeinfo>
#include <any>

namespace gm::core {

/// Type-erased device interface
class Device {
public:
    template<DeviceLike T>
    Device(T&& impl) : m_impl(std::make_unique<Model<std::decay_t<T>>>(std::forward<T>(impl))) {}
    
    Device(const Device& other) : m_impl(other.m_impl->clone()) {}
    Device& operator=(const Device& other) {
        if (this != &other) {
            m_impl = other.m_impl->clone();
        }
        return *this;
    }
    
    Device(Device&&) = default;
    Device& operator=(Device&&) = default;
    
    auto IsConnected() const -> bool { return m_impl->IsConnected(); }
    auto Connect() -> VoidResult { return m_impl->Connect(); }
    auto Disconnect() -> VoidResult { return m_impl->Disconnect(); }
    auto Process() -> VoidResult { return m_impl->Process(); }
    auto GetName() const -> std::string_view { return m_impl->GetName(); }
    auto GetGUID() const -> GUID { return m_impl->GetGUID(); }
    auto IsValid() const -> bool { return m_impl->IsValid(); }
    auto GetLastProcessTime() const -> std::chrono::time_point<std::chrono::steady_clock> { 
        return m_impl->GetLastProcessTime(); 
    }
    
    template<typename T>
    auto get_if() -> T* {
        return m_impl->template get_if<T>();
    }
    
    template<typename T>
    auto get_if() const -> const T* {
        return m_impl->template get_if<T>();
    }
    
    auto type_info() const -> const std::type_info& {
        return m_impl->type_info();
    }

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual auto clone() const -> std::unique_ptr<Concept> = 0;
        virtual auto IsConnected() const -> bool = 0;
        virtual auto Connect() -> VoidResult = 0;
        virtual auto Disconnect() -> VoidResult = 0;
        virtual auto Process() -> VoidResult = 0;
        virtual auto GetName() const -> std::string_view = 0;
        virtual auto GetGUID() const -> GUID = 0;
        virtual auto IsValid() const -> bool = 0;
        virtual auto GetLastProcessTime() const -> std::chrono::time_point<std::chrono::steady_clock> = 0;
        virtual auto type_info() const -> const std::type_info& = 0;
        
        template<typename T>
        auto get_if() -> T* { return nullptr; }
        
        template<typename T>
        auto get_if() const -> const T* { return nullptr; }
    };
    
    template<DeviceLike T>
    struct Model : Concept {
        explicit Model(T impl) : m_impl(std::move(impl)) {}
        
        auto clone() const -> std::unique_ptr<Concept> override {
            return std::make_unique<Model>(*this);
        }
        
        auto IsConnected() const -> bool override { return m_impl.IsConnected(); }
        auto Connect() -> VoidResult override { return m_impl.Connect(); }
        auto Disconnect() -> VoidResult override { return m_impl.Disconnect(); }
        auto Process() -> VoidResult override { return m_impl.Process(); }
        auto GetName() const -> std::string_view override { return m_impl.GetName(); }
        auto GetGUID() const -> GUID override { return m_impl.GetGUID(); }
        auto IsValid() const -> bool override { return m_impl.IsValid(); }
        auto GetLastProcessTime() const -> std::chrono::time_point<std::chrono::steady_clock> override { 
            return m_impl.GetLastProcessTime(); 
        }
        auto type_info() const -> const std::type_info& override { return typeid(T); }
        
        template<typename U>
        auto get_if() -> U* {
            if constexpr (std::is_same_v<U, T>) {
                return &m_impl;
            }
            return nullptr;
        }
        
        template<typename U>
        auto get_if() const -> const U* {
            if constexpr (std::is_same_v<U, T>) {
                return &m_impl;
            }
            return nullptr;
        }
        
        T m_impl;
    };
    
    std::unique_ptr<Concept> m_impl;
};

/// Type-erased configuration source
class ConfigSource {
public:
    template<ConfigurationSource T>
    ConfigSource(T&& impl) : m_impl(std::make_unique<Model<std::decay_t<T>>>(std::forward<T>(impl))) {}
    
    ConfigSource(const ConfigSource& other) : m_impl(other.m_impl->clone()) {}
    ConfigSource& operator=(const ConfigSource& other) {
        if (this != &other) {
            m_impl = other.m_impl->clone();
        }
        return *this;
    }
    
    ConfigSource(ConfigSource&&) = default;
    ConfigSource& operator=(ConfigSource&&) = default;
    
    auto Load() -> VoidResult { return m_impl->Load(); }
    auto Save() -> VoidResult { return m_impl->Save(); }
    auto IsLoaded() const -> bool { return m_impl->IsLoaded(); }
    
    template<typename T>
    auto get_if() -> T* { return m_impl->template get_if<T>(); }
    
    template<typename T>
    auto get_if() const -> const T* { return m_impl->template get_if<T>(); }

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual auto clone() const -> std::unique_ptr<Concept> = 0;
        virtual auto Load() -> VoidResult = 0;
        virtual auto Save() -> VoidResult = 0;
        virtual auto IsLoaded() const -> bool = 0;
        
        template<typename T>
        auto get_if() -> T* { return nullptr; }
        
        template<typename T>
        auto get_if() const -> const T* { return nullptr; }
    };
    
    template<ConfigurationSource T>
    struct Model : Concept {
        explicit Model(T impl) : m_impl(std::move(impl)) {}
        
        auto clone() const -> std::unique_ptr<Concept> override {
            return std::make_unique<Model>(*this);
        }
        
        auto Load() -> VoidResult override { return m_impl.Load(); }
        auto Save() -> VoidResult override { return m_impl.Save(); }
        auto IsLoaded() const -> bool override { return m_impl.IsLoaded(); }
        
        template<typename U>
        auto get_if() -> U* {
            if constexpr (std::is_same_v<U, T>) {
                return &m_impl;
            }
            return nullptr;
        }
        
        template<typename U>
        auto get_if() const -> const U* {
            if constexpr (std::is_same_v<U, T>) {
                return &m_impl;
            }
            return nullptr;
        }
        
        T m_impl;
    };
    
    std::unique_ptr<Concept> m_impl;
};

/// Function wrapper with type erasure
template<typename Signature>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)> {
public:
    template<typename F>
        requires std::invocable<F, Args...> && 
                 std::convertible_to<std::invoke_result_t<F, Args...>, R>
    Function(F&& f) : m_impl(std::make_unique<Model<std::decay_t<F>>>(std::forward<F>(f))) {}
    
    Function(const Function& other) : m_impl(other.m_impl ? other.m_impl->clone() : nullptr) {}
    Function& operator=(const Function& other) {
        if (this != &other) {
            m_impl = other.m_impl ? other.m_impl->clone() : nullptr;
        }
        return *this;
    }
    
    Function(Function&&) = default;
    Function& operator=(Function&&) = default;
    
    auto operator()(Args... args) const -> R {
        return m_impl->call(std::forward<Args>(args)...);
    }
    
    explicit operator bool() const noexcept {
        return static_cast<bool>(m_impl);
    }

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual auto clone() const -> std::unique_ptr<Concept> = 0;
        virtual auto call(Args... args) const -> R = 0;
    };
    
    template<typename F>
    struct Model : Concept {
        explicit Model(F f) : m_f(std::move(f)) {}
        
        auto clone() const -> std::unique_ptr<Concept> override {
            return std::make_unique<Model>(*this);
        }
        
        auto call(Args... args) const -> R override {
            return m_f(std::forward<Args>(args)...);
        }
        
        F m_f;
    };
    
    std::unique_ptr<Concept> m_impl;
};

} // namespace gm::core