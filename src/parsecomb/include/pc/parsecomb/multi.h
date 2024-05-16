#pragma once

#include "pc/parsecomb/types.h"

#include <expected>
#include <type_traits>
#include <vector>

namespace pc {

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
    return fold_many_l_h(0, pc::InfMany, parser, init, f);
}

template<typename P, typename T, typename F>
auto fold_many_more(size_t lower, P parser, T init, F f) {
    return fold_many_l_h(lower, pc::InfMany, parser, init, f);
}

template<typename P>
auto many_lower_upper(size_t lower, size_t upper, P parser) {
    using ItemType = std::invoke_result_t<P, StringRef &>::value_type;

    return fold_many_lower_upper(lower, upper, parser, std::vector<ItemType>{},
                                 [](auto &accumulator, auto &&new_element) {
                                     accumulator.push_back(std::forward<decltype(new_element)>(new_element));
                                 });
}

template<typename P>
auto many_any(P parser) {
    return many_lower_upper(0, pc::InfMany, parser);
}

template<typename P>
auto many_more(size_t lower, P parser) {
    return many_lower_upper(lower, pc::InfMany, parser);
}

}// namespace pc
