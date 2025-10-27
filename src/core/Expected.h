#pragma once
#include <expected>
#include <system_error>
#include <string_view>
#include <format>

namespace gm::core {

/// Modern error handling with std::expected
enum class ErrorCode {
    Success = 0,
    InvalidArgument,
    ResourceNotFound,
    AccessDenied,
    DeviceNotConnected,
    ConfigurationError,
    DirectInputError,
    UnknownError
};

struct Error {
    ErrorCode code;
    std::string_view message;
    std::source_location location = std::source_location::current();
    
    constexpr Error(ErrorCode c, std::string_view msg) noexcept 
        : code(c), message(msg) {}
    
    [[nodiscard]] auto what() const -> std::string {
        return std::format("Error[{}]: {} at {}:{}",
            static_cast<int>(code), message, 
            location.file_name(), location.line());
    }
};

/// Convenience aliases for common Result types
template<typename T>
using Result = std::expected<T, Error>;

using VoidResult = Result<void>;

/// Error creation helpers
constexpr auto make_error(ErrorCode code, std::string_view message) -> Error {
    return Error{code, message};
}

constexpr auto invalid_argument(std::string_view msg) -> Error {
    return make_error(ErrorCode::InvalidArgument, msg);
}

constexpr auto resource_not_found(std::string_view msg) -> Error {
    return make_error(ErrorCode::ResourceNotFound, msg);
}

constexpr auto access_denied(std::string_view msg) -> Error {
    return make_error(ErrorCode::AccessDenied, msg);
}

constexpr auto device_error(std::string_view msg) -> Error {
    return make_error(ErrorCode::DeviceNotConnected, msg);
}

constexpr auto config_error(std::string_view msg) -> Error {
    return make_error(ErrorCode::ConfigurationError, msg);
}

constexpr auto directinput_error(std::string_view msg) -> Error {
    return make_error(ErrorCode::DirectInputError, msg);
}

/// Monadic operations for Result composition
template<typename T, typename F>
constexpr auto and_then(Result<T>&& result, F&& func) -> decltype(func(std::move(*result))) {
    if (result.has_value()) {
        return func(std::move(*result));
    }
    return std::unexpected(std::move(result.error()));
}

template<typename T, typename F>
constexpr auto transform(Result<T>&& result, F&& func) -> Result<decltype(func(std::move(*result)))> {
    if (result.has_value()) {
        return func(std::move(*result));
    }
    return std::unexpected(std::move(result.error()));
}

template<typename T, typename F>
constexpr auto or_else(Result<T>&& result, F&& func) -> Result<T> {
    if (!result.has_value()) {
        return func(std::move(result.error()));
    }
    return std::move(result);
}

/// HRESULT to Result conversion
inline auto from_hresult(HRESULT hr, std::string_view context = "") -> VoidResult {
    if (SUCCEEDED(hr)) {
        return {};
    }
    
    auto message = std::format("HRESULT failure: 0x{:08X}", hr);
    if (!context.empty()) {
        message += std::format(" in {}", context);
    }
    
    return std::unexpected(directinput_error(message));
}

template<typename T>
auto from_hresult(HRESULT hr, T&& value, std::string_view context = "") -> Result<T> {
    if (SUCCEEDED(hr)) {
        return std::forward<T>(value);
    }
    
    auto message = std::format("HRESULT failure: 0x{:08X}", hr);
    if (!context.empty()) {
        message += std::format(" in {}", context);
    }
    
    return std::unexpected(directinput_error(message));
}

} // namespace gm::core