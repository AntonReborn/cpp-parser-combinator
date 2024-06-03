#pragma once

#include "pc/parsecomb/types.h"
#include "pc/parsecomb/functional.h"

#include <type_traits>
#include <tuple>

namespace pc {

template<typename Parser>
auto tuple(Parser parser) {
    using ValueT = std::tuple<typename std::invoke_result_t<Parser, StringRef &>::value_type>;
    return [parser](StringRef &input) -> ParseResult<ValueT> {
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

    return [p_head, p_tail...](StringRef &input) -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);

        return p_head(input).and_then([&](auto head_value) -> ParseResult<ValueT> {
            return tuple(p_tail...)(input).transform([&](auto tail_value) {
                return guard.build(std::tuple_cat(std::make_tuple(head_value), tail_value));
            });
        });
    };
}

template<std::size_t Idx, typename... Parsers>
auto take(Parsers... parsers) {
    return map(tuple(parsers...), [](const auto& tp) { return std::get<Idx>(tp); });
}

template<std::size_t Idx1, std::size_t Idx2, typename... Parsers>
auto take(Parsers... parsers) {
    return map(tuple(parsers...), [](const auto& tp) { return std::make_tuple(std::get<Idx1>(tp), std::get<Idx2>(tp)); });
}

}// namespace pc
