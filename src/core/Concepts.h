#pragma once
#include <concepts>
#include <type_traits>
#include <chrono>
#include <string_view>
#include <windows.h>
#include <dinput.h>

namespace gm::core {

/// Device-related concepts
template<typename T>
concept Connectable = requires(T t) {
    { t.IsConnected() } -> std::convertible_to<bool>;
    { t.Connect() } -> std::same_as<VoidResult>;
    { t.Disconnect() } -> std::same_as<VoidResult>;
};

template<typename T>
concept Processable = requires(T t) {
    { t.Process() } -> std::same_as<VoidResult>;
    { t.GetLastProcessTime() } -> std::convertible_to<std::chrono::time_point<std::chrono::steady_clock>>;
};

template<typename T>
concept DeviceLike = Connectable<T> && Processable<T> && requires(T t) {
    { t.GetName() } -> std::convertible_to<std::string_view>;
    { t.GetGUID() } -> std::convertible_to<GUID>;
    { t.IsValid() } -> std::convertible_to<bool>;
};

/// Input-related concepts
template<typename T>
concept InputState = requires(T t) {
    typename T::button_type;
    typename T::axis_type;
    { t.GetButtonState(0) } -> std::convertible_to<bool>;
    { t.GetAxisValue(typename T::axis_type{}) } -> std::convertible_to<float>;
};

template<typename T>
concept KeyMappable = requires(T t) {
    { t.MapToKeys() } -> std::convertible_to<std::span<const WORD>>;
    { t.IsActive() } -> std::convertible_to<bool>;
};

template<typename T>
concept ConfigurationSource = requires(T t) {
    { t.Load() } -> std::same_as<VoidResult>;
    { t.Save() } -> std::same_as<VoidResult>;
    { t.IsLoaded() } -> std::convertible_to<bool>;
};

/// Logging concepts
template<typename T>
concept Loggable = requires(T t) {
    { t.GetLogLevel() } -> std::convertible_to<int>;
    { t.FormatMessage() } -> std::convertible_to<std::string_view>;
};

template<typename T>
concept LogSink = requires(T t, std::string_view msg) {
    t.Write(msg);
    { t.ShouldLog(0) } -> std::convertible_to<bool>;
};

/// Memory management concepts
template<typename T>
concept Poolable = requires {
    typename T::pool_tag;
    { T::pool_size } -> std::convertible_to<std::size_t>;
} && std::is_trivially_destructible_v<T>;

template<typename T>
concept ZeroCopyMovable = std::is_trivially_move_constructible_v<T> 
                       && std::is_trivially_move_assignable_v<T>
                       && !std::is_copy_constructible_v<T>;

/// Functional concepts
template<typename F, typename... Args>
concept Invocable = std::invocable<F, Args...>;

template<typename F, typename R, typename... Args>
concept InvocableR = std::invocable<F, Args...> && 
                     std::convertible_to<std::invoke_result_t<F, Args...>, R>;

template<typename T>
concept Monadic = requires(T t) {
    typename T::value_type;
    typename T::error_type;
    { t.has_value() } -> std::convertible_to<bool>;
    { *t } -> std::convertible_to<typename T::value_type>;
    { t.error() } -> std::convertible_to<typename T::error_type>;
};

/// Compile-time configuration concepts
template<typename T>
concept StaticConfiguration = requires {
    { T::button_count } -> std::convertible_to<std::size_t>;
    { T::axis_count } -> std::convertible_to<std::size_t>;
    { T::device_name } -> std::convertible_to<std::string_view>;
};

template<auto Config>
concept ValidDeviceConfig = StaticConfiguration<decltype(Config)> && requires {
    Config.button_count > 0;
    Config.axis_count > 0;
    !Config.device_name.empty();
};

/// CRTP concepts
template<typename Derived, typename Base>
concept CRTPDerived = std::is_base_of_v<Base<Derived>, Derived>;

template<typename T>
concept NonVirtual = !std::is_polymorphic_v<T>;

/// Performance concepts
template<typename T>
concept CacheEfficient = sizeof(T) <= 64 && alignof(T) <= 64; // Cache line friendly

template<typename T>
concept LockFree = requires(T t) {
    { T::is_always_lock_free } -> std::convertible_to<bool>;
} && T::is_always_lock_free;

/// Type erasure helper concepts
template<typename T, typename Interface>
concept ImplementsInterface = requires {
    static_cast<Interface*>(static_cast<T*>(nullptr));
};

template<typename T>
concept TypeErased = requires(T t) {
    { t.type_info() } -> std::convertible_to<std::type_info>;
    { t.get_if<int>() } -> std::convertible_to<int*>;
};

} // namespace gm::core