#pragma once

#include "pc/parsecomb/types.h"

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

template<typename Open, typename Parser, typename Close>
auto discard_surround(Open open_parser, Parser parser, Close close_parser) {
    using ValueT = std::invoke_result_t<Parser, StringRef &>::value_type;
    return [open_parser, parser, close_parser](StringRef &input) -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        return open_parser(input)
                .and_then([&](auto) { return parser(input); })
                .and_then([&](auto to_return) {
                    return close_parser(input).transform([&to_return, &guard](auto) { return guard.build(std::move(to_return)); });
                });
    };
}

template<typename Preceed, typename Parser>
auto discard_preceded(Preceed to_discard, Parser parser) {
    using ValueT = std::invoke_result_t<Parser, StringRef &>::value_type;
    return [to_discard, parser](StringRef &input) -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        return to_discard(input)
                .and_then([&](auto) { guard.keep(); return parser(input); });
    };
}

template<typename Parser, typename Terminate>
auto discard_terminated(Parser parser, Terminate to_discard) {
    using ValueT = std::invoke_result_t<Parser, StringRef &>::value_type;
    return [parser, to_discard](StringRef &input) -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        return parser(input)
                .and_then([&](auto to_return) { 
                    return to_discard(input).transform([&to_return, &guard](auto) { return guard.build(std::move(to_return)); });
        });
    };
}
}// namespace pc
