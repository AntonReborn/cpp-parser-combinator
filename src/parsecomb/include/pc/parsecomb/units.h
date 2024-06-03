#pragma once

#include "pc/parsecomb/types.h"
#include "pc/parsecomb/traits.h"
#include <tuple>
#include <type_traits>

namespace pc {

template<typename Parser, typename EmitT>
auto to(Parser p, EmitT emit) {
    return [p, emit](StringRef& input) -> ParseResult<decltype(emit)> {
        PC_EXPECT(p(input));
        return emit;
    };
}

template<typename Parser, typename F>
auto map(Parser p, F f) {
    return [p, f](StringRef& input) -> ParseResult<std::invoke_result_t<F, typename std::invoke_result_t<Parser, StringRef&>::value_type>> {
        PC_EXPECT_ASSIGN(result, p(input));

        return f(result);
    };
}

inline auto char_range(char from, char to) {
    return [from, to](StringRef &input) -> ParseResult<char> {
        ResultBuilder<char> guard(input);
        if (input.empty())
            return std::unexpected(ParseError::Unknown);

        if (input.front() >= from && input.front() <= to) {
            auto result = input.front();
            input.remove_prefix(1);
            return guard.build(result);
        } else {
            return std::unexpected(ParseError::Unknown);
        }
    };
}

inline auto char_exact(char ch) {
    return char_range(ch, ch);
}

inline auto tag(StringRef item) {
    return [item](StringRef &input) -> ParseResult<NothingT> {
        auto item_copy = item;
        ResultBuilder<NothingT> guard(input);

        if (input.size() < item_copy.size())
            return std::unexpected(ParseError::Unknown);

        while (!item_copy.empty() && input.front() == item_copy.front()) {
            input.remove_prefix(1);
            item_copy.remove_prefix(1);
        }

        if (!item_copy.empty())
            return std::unexpected(ParseError::Unknown);

        return guard.build(Nothing);
    };
}


inline auto spaces(size_t n) {
    return [n](StringRef &input) -> ParseResult<NothingT> {
        ResultBuilder<NothingT> guard(input);
        auto n_copy = n;
        if (input.size() < n_copy)
            return std::unexpected(ParseError::Unknown);

        while (std::isspace(input.front())) {
            input.remove_prefix(1);

            if (n_copy)
                n_copy--;
        }

        if (n_copy)
            return std::unexpected(ParseError::Unknown);

        return guard.build(Nothing);
    };
}

inline auto int32() {
    return [](StringRef &input) -> ParseResult<std::int32_t> {
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
}// namespace pc
