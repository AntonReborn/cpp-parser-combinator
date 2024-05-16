#pragma once

#include <expected>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <variant>

using namespace std;

namespace pc {

using StringRef = string_view;
using NothingT = std::monostate;
constexpr NothingT Nothing = NothingT{};

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

    T build(T value) {
        parsed_ = true;

        return value;
    }

    bool parsed_ = false;

    StringRef &view_;
    StringRef original_;
};

auto tag(StringRef item) {
    return [item](StringRef &input) mutable -> ParseResult<NothingT> {
        ResultBuilder<NothingT> guard(input);

        if (input.size() < item.size())
            return std::unexpected(ParseError::Unknown);

        while (!item.empty() && input.front() == item.front()) {
            input.remove_prefix(1);
            item.remove_prefix(1);
        }

        if (!item.empty())
            return std::unexpected(ParseError::Unknown);

        return guard.build(Nothing);
    };
}

auto spaces(size_t n) {
    return [n](StringRef &input) mutable -> ParseResult<NothingT> {
        ResultBuilder<NothingT> guard(input);
        if (input.size() < n)
            return std::unexpected(ParseError::Unknown);

        while (std::isspace(input.front())) {
            input.remove_prefix(1);

            if (n)
                n--;
        }

        if (n)
            return std::unexpected(ParseError::Unknown);

        return guard.build(Nothing);
    };
}

auto int32() {
    return [](StringRef &input) mutable -> ParseResult<std::int32_t> {
        ResultBuilder<std::int32_t> guard(input);

        if (input.empty())
            return std::unexpected(ParseError::Unknown);

        std::int32_t sign = 1;
        if (input.front() == '-') {
            sign = -1;
            input.remove_prefix(1);
        }

        std::int32_t value = 0;
        if (input.empty() || !std::isdigit(input.front()))
            return std::unexpected(ParseError::Unknown);

        while (std::isdigit(input.front())) {
            value *= 10;
            value += input.front() - '0';
            input.remove_prefix(1);
        }

        return guard.build(value * sign);
    };
}

template<typename P>
auto tuple(P parser) {
    using ValueT = std::tuple<typename std::invoke_result_t<P, StringRef &>::value_type>;
    return [parser](StringRef &input) mutable -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        return parser(input).and_then([&guard](auto head_value) -> ParseResult<ValueT> {
            return guard.build(std::make_tuple(head_value));
        });
    };
}

template<typename HeadP, typename... TailPs>
auto tuple(HeadP p_head, TailPs... p_tail) {
    using ValueT = std::tuple<typename std::invoke_result_t<HeadP, StringRef &>::value_type,
                              typename std::invoke_result_t<TailPs, StringRef &>::value_type...>;

    return [p_head, p_tail...](StringRef &input) mutable -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);

        return p_head(input).and_then([&](auto head_value) -> ParseResult<ValueT> {
            return tuple(p_tail...)(input).transform([&](auto tail_value) {
                return guard.build(std::tuple_cat(std::make_tuple(head_value), tail_value));
            });
        });
    };
}

template<typename OP, typename P, typename CP>
auto delimeted(OP open_parser, P parser, CP close_parser) {
    using ValueT = std::invoke_result_t<P, StringRef &>::value_type;
    return [open_parser, parser, close_parser](StringRef &input) mutable -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        return open_parser(input)
                .and_then([&](auto) { return parser(input); })
                .and_then([&](auto to_return) {
                    return close_parser(input).transform([&to_return, &guard](auto) { return guard.build(std::move(to_return)); });
                });
    };
}

#define PC_LEAF_ASSIGN(var, exp)                    \
    auto &&var##_temp = exp;                        \
    if (!var##_temp.has_value()) {                  \
        return std::unexpected(var##_temp.error()); \
    }                                               \
    auto var = std::forward<decltype(var##_temp)>(var##_temp).value();\
    (void)var

}// namespace pc
