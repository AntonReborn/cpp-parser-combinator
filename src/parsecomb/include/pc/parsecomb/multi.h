#pragma once

#include "pc/parsecomb/types.h"
#include "pc/parsecomb/units.h"
#include "sequence.h"

#include <expected>
#include <type_traits>
#include <vector>

namespace pc {

// TODO: parametrize policy, GREEDY right now.
template<typename P, typename T, typename F>
auto fold_many_lower_upper(size_t lower, size_t upper, P parser, T init, F f) {
    return [lower, upper, parser, init, f](StringRef &input) -> ParseResult<T> {
        ResultBuilder<T> guard(input);
        auto result = init;
        size_t i = 0;

        for (; i < upper; i++) {
            auto one_more = parser(input);
            if (one_more)
                f(result, one_more.value());
            else
                break;
        }

        if (i < lower)
            return std::unexpected(ParseError::Unknown);

        return guard.build(std::move(result));
    };
}

template<typename P, typename T, typename F>
auto fold_many_any(P parser, T init, F f) {
    return fold_many_lower_upper(0, pc::InfMany, parser, init, f);
}

template<typename P, typename T, typename F>
auto fold_many_more(size_t lower, P parser, T init, F f) {
    return fold_many_lower_upper(lower, pc::InfMany, parser, init, f);
}

template<typename P>
auto many_lower_upper(size_t lower, size_t upper, P parser) {
    using ItemType = typename std::invoke_result_t<P, StringRef &>::value_type;

    return fold_many_lower_upper(lower, upper, parser, std::vector<ItemType>{},
                                 [](auto &accumulator, auto&&new_element) {
                                     accumulator.push_back(std::forward<decltype(new_element)>(new_element));
                                 });
}

template<typename P>
auto many_any(P parser) {
    return many_lower_upper(0, InfMany, parser);
}

template<typename P>
auto many_more(size_t lower, P parser) {
    return many_lower_upper(lower, InfMany, parser);
}

// TODO: optimize
template<typename P>
auto maybe_ignore(P parser) {
    return pc::to(fold_many_lower_upper(0, 1, parser, 0, [](int, auto) { return 0; }), Nothing);
}

template<typename P, typename S, typename T, typename F>
auto fold_any_separated_by(P parser, S separator, T init, F f) {
    return [parser, separator, init, f](StringRef &input) -> ParseResult<T> {
        ResultBuilder<T> guard(input);

        PC_EXPECT_ASSIGN(prefix, fold_many_any(pc::discard_terminated(parser, separator), init, f)(input));
        PC_EXPECT_ASSIGN(suffix, parser(input));

        f(prefix, suffix);

        return guard.build(std::move(prefix));
    };
}

template<typename P, typename S>
auto many_any_separated_by(P parser, S separator) {
    using ItemType = typename std::invoke_result_t<P, StringRef &>::value_type;

    return fold_any_separated_by(parser, separator, std::vector<ItemType>{}, [](auto &accumulator, auto &&new_element) {
        accumulator.push_back(std::forward<decltype(new_element)>(new_element));
    });
}

}// namespace pc
