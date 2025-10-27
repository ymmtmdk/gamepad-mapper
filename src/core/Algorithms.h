#pragma once
#include "Expected.h"
#include "Concepts.h"
#include <ranges>
#include <algorithm>
#include <functional>
#include <numeric>

namespace gm::core {

/// Modern algorithm utilities with ranges and monadic operations
namespace algo {

/// Transform with error handling
template<std::ranges::input_range R, typename F>
    requires std::invocable<F, std::ranges::range_value_t<R>>
auto transform_result(R&& range, F&& func) -> Result<std::vector<std::invoke_result_t<F, std::ranges::range_value_t<R>>>> {
    using ValueType = std::invoke_result_t<F, std::ranges::range_value_t<R>>;
    std::vector<ValueType> result;
    result.reserve(std::ranges::size(range));
    
    for (auto&& element : range) {
        if constexpr (Monadic<std::invoke_result_t<F, decltype(element)>>) {
            if (auto transformed = func(std::forward<decltype(element)>(element)); transformed) {
                result.emplace_back(std::move(*transformed));
            } else {
                return std::unexpected(std::move(transformed.error()));
            }
        } else {
            result.emplace_back(func(std::forward<decltype(element)>(element)));
        }
    }
    
    return result;
}

/// Filter with predicate
template<std::ranges::input_range R, typename P>
    requires std::predicate<P, std::ranges::range_value_t<R>>
auto filter(R&& range, P&& predicate) {
    return std::forward<R>(range) | std::views::filter(std::forward<P>(predicate));
}

/// Partition into valid and invalid elements
template<std::ranges::input_range R, typename P>
    requires std::predicate<P, std::ranges::range_value_t<R>>
auto partition_results(R&& range, P&& is_valid) -> std::pair<std::vector<std::ranges::range_value_t<R>>, 
                                                              std::vector<std::ranges::range_value_t<R>>> {
    std::vector<std::ranges::range_value_t<R>> valid, invalid;
    
    for (auto&& element : range) {
        if (is_valid(element)) {
            valid.emplace_back(std::forward<decltype(element)>(element));
        } else {
            invalid.emplace_back(std::forward<decltype(element)>(element));
        }
    }
    
    return {std::move(valid), std::move(invalid)};
}

/// Find first successful result
template<std::ranges::input_range R, typename F>
    requires std::invocable<F, std::ranges::range_value_t<R>>
auto find_first_success(R&& range, F&& func) -> std::optional<std::invoke_result_t<F, std::ranges::range_value_t<R>>> {
    for (auto&& element : range) {
        if (auto result = func(std::forward<decltype(element)>(element)); result) {
            return result;
        }
    }
    return std::nullopt;
}

/// Parallel processing with error aggregation
template<std::ranges::input_range R, typename F>
    requires std::invocable<F, std::ranges::range_value_t<R>>
auto parallel_transform(R&& range, F&& func) -> Result<std::vector<std::invoke_result_t<F, std::ranges::range_value_t<R>>>> {
    // For now, implement as sequential (can be upgraded to std::execution later)
    return transform_result(std::forward<R>(range), std::forward<F>(func));
}

/// Reduce with early termination on error
template<std::ranges::input_range R, typename T, typename F>
    requires std::invocable<F, T, std::ranges::range_value_t<R>>
auto reduce_result(R&& range, T init, F&& func) -> Result<T> {
    T accumulator = std::move(init);
    
    for (auto&& element : range) {
        if constexpr (Monadic<std::invoke_result_t<F, T, decltype(element)>>) {
            if (auto result = func(std::move(accumulator), std::forward<decltype(element)>(element)); result) {
                accumulator = std::move(*result);
            } else {
                return std::unexpected(std::move(result.error()));
            }
        } else {
            accumulator = func(std::move(accumulator), std::forward<decltype(element)>(element));
        }
    }
    
    return accumulator;
}

/// Chain multiple operations with short-circuit evaluation
template<typename T, typename... Fs>
auto chain_operations(T&& initial, Fs&&... operations) -> Result<T> {
    Result<T> current = std::forward<T>(initial);
    
    auto apply_operation = [&current](auto&& op) {
        if (current.has_value()) {
            current = op(std::move(*current));
        }
    };
    
    (apply_operation(operations), ...);
    return current;
}

/// Batch processing with configurable batch size
template<std::ranges::input_range R, typename F>
    requires std::invocable<F, std::vector<std::ranges::range_value_t<R>>>
auto batch_process(R&& range, std::size_t batch_size, F&& processor) -> VoidResult {
    std::vector<std::ranges::range_value_t<R>> batch;
    batch.reserve(batch_size);
    
    for (auto&& element : range) {
        batch.emplace_back(std::forward<decltype(element)>(element));
        
        if (batch.size() >= batch_size) {
            if (auto result = processor(batch); !result) {
                return result;
            }
            batch.clear();
        }
    }
    
    // Process remaining elements
    if (!batch.empty()) {
        return processor(batch);
    }
    
    return {};
}

} // namespace algo

/// Functional composition utilities
namespace compose {

/// Function composition operator
template<typename F, typename G>
constexpr auto operator|(F&& f, G&& g) {
    return [f = std::forward<F>(f), g = std::forward<G>(g)](auto&&... args) {
        return g(f(std::forward<decltype(args)>(args)...));
    };
}

/// Bind front arguments
template<typename F, typename... Args>
constexpr auto bind_front(F&& f, Args&&... args) {
    return [f = std::forward<F>(f), ...bound_args = std::forward<Args>(args)](auto&&... remaining_args) {
        return f(bound_args..., std::forward<decltype(remaining_args)>(remaining_args)...);
    };
}

/// Curry a function
template<typename F>
constexpr auto curry(F&& f) {
    return [f = std::forward<F>(f)](auto&& first_arg) {
        return [f, first_arg = std::forward<decltype(first_arg)>(first_arg)](auto&&... rest_args) {
            return f(first_arg, std::forward<decltype(rest_args)>(rest_args)...);
        };
    };
}

/// Memoization wrapper
template<typename F>
class Memoized {
public:
    explicit Memoized(F f) : m_func(std::move(f)) {}
    
    template<typename... Args>
    auto operator()(Args&&... args) -> std::invoke_result_t<F, Args...> {
        auto key = std::make_tuple(std::forward<Args>(args)...);
        
        if (auto it = m_cache.find(key); it != m_cache.end()) {
            return it->second;
        }
        
        auto result = m_func(std::forward<Args>(args)...);
        m_cache[std::move(key)] = result;
        return result;
    }

private:
    F m_func;
    std::map<std::tuple<std::decay_t<Args>...>, std::invoke_result_t<F, Args...>> m_cache;
};

template<typename F>
auto memoize(F&& f) -> Memoized<std::decay_t<F>> {
    return Memoized<std::decay_t<F>>(std::forward<F>(f));
}

} // namespace compose

/// Monadic utilities for Result types
namespace monadic {

/// Lift a regular function to work with Result types
template<typename F>
constexpr auto lift(F&& func) {
    return [func = std::forward<F>(func)](auto&& result) -> Result<std::invoke_result_t<F, decltype(*result)>> {
        if (result.has_value()) {
            return func(*std::forward<decltype(result)>(result));
        }
        return std::unexpected(std::forward<decltype(result)>(result).error());
    };
}

/// Apply a function to multiple Result arguments
template<typename F, typename... Results>
constexpr auto apply(F&& func, Results&&... results) -> Result<std::invoke_result_t<F, decltype(*results)...>> {
    // Check if all results are valid
    if ((results.has_value() && ...)) {
        return func(*std::forward<Results>(results)...);
    }
    
    // Return the first error encountered
    Error first_error = invalid_argument("Multiple errors in apply");
    ((results.has_value() ? void() : (first_error = results.error(), void())), ...);
    return std::unexpected(first_error);
}

/// Sequence a container of Results into a Result of container
template<std::ranges::input_range R>
    requires Monadic<std::ranges::range_value_t<R>>
auto sequence(R&& results) -> Result<std::vector<typename std::ranges::range_value_t<R>::value_type>> {
    using ValueType = typename std::ranges::range_value_t<R>::value_type;
    std::vector<ValueType> values;
    values.reserve(std::ranges::size(results));
    
    for (auto&& result : results) {
        if (result.has_value()) {
            values.emplace_back(*std::forward<decltype(result)>(result));
        } else {
            return std::unexpected(std::forward<decltype(result)>(result).error());
        }
    }
    
    return values;
}

/// Traverse: map and sequence in one operation
template<std::ranges::input_range R, typename F>
    requires std::invocable<F, std::ranges::range_value_t<R>> &&
             Monadic<std::invoke_result_t<F, std::ranges::range_value_t<R>>>
auto traverse(R&& range, F&& func) -> Result<std::vector<typename std::invoke_result_t<F, std::ranges::range_value_t<R>>::value_type>> {
    auto transformed = std::forward<R>(range) | std::views::transform(std::forward<F>(func));
    return sequence(transformed);
}

} // namespace monadic

} // namespace gm::core