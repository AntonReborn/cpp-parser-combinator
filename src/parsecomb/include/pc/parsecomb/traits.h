#pragma once
#include <concepts>
#include <type_traits>

namespace pc {

template<typename H, typename... Ts>
struct first {
    using type = H;
};

template<typename T>
concept Eq = requires(T a, T b)
{
    a == b;
};

template<typename T, typename... Ts>
concept AllSame = (std::same_as<T, Ts> && ...);

template<typename... Ts>
struct has_duplicates_with;

template<typename T>
struct has_duplicates_with<T> : std::false_type {};

template<typename T, typename... Ts>
struct has_duplicates_with<T, Ts...> {
    static constexpr bool value = (std::same_as<T, Ts> || ...) || has_duplicates_with<Ts...>::value;
};

static_assert(!has_duplicates_with<int, float, double>::value, "has_duplicates_with test failed");
static_assert(!has_duplicates_with<int>::value, "has_duplicates_with test failed");
static_assert(has_duplicates_with<int, float, double, float>::value, "has_duplicates_with test failed");
static_assert(has_duplicates_with<int, int>::value, "has_duplicates_with test failed");
static_assert(!has_duplicates_with<int>::value, "has_duplicates_with test failed");
static_assert(!has_duplicates_with<std::string, int>::value, "has_duplicates_with test failed");

template<typename... Ts>
concept AllUnique = !has_duplicates_with<Ts...>::value;

void print_typename(auto) requires false {}
}// namespace pc
