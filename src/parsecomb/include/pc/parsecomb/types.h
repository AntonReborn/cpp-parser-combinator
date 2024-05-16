#pragma once

#include <expected>
#include <limits>
#include <string_view>
#include <variant>

#define PC_LEAF_ASSIGN(var, exp)                                       \
    auto &&var##_temp = exp;                                           \
    if (!var##_temp.has_value()) {                                     \
        return std::unexpected(var##_temp.error());                    \
    }                                                                  \
    auto var = std::forward<decltype(var##_temp)>(var##_temp).value(); \
    (void) var

namespace pc {

using StringRef = std::string_view;
using NothingT = std::monostate;
constexpr NothingT Nothing = NothingT{};

constexpr size_t InfMany = std::numeric_limits<size_t>::max();

enum class ParseError {
    Unknown,
};

template<typename T>
using ParseResult = std::expected<T, ParseError>;

template<typename T>
struct ResultBuilder {
    ResultBuilder(StringRef &view)
        : view_(view), original_(view) {}

    ResultBuilder(const ResultBuilder &) = delete;
    ResultBuilder(ResultBuilder &&) = delete;

    ~ResultBuilder() {
        if (!parsed_)
            view_ = original_;
    }

    /* unsafe */
    void keep() {
        parsed_ = true;
    }

    // TODO: Rely on type system
    T build(T value) {
        parsed_ = true;

        return value;
    }

    bool parsed_ = false;

    StringRef &view_;
    StringRef original_;
};
}// namespace pc
