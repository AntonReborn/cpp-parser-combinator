#pragma once

#include "pc/parsecomb/types.h"
#include "traits.h"

#include <type_traits>
#include <variant>

namespace pc {


template<typename P>
auto alt(P parser) {
    using ValueT = typename std::invoke_result_t<P, StringRef &>::value_type;

    return [parser](StringRef &input) -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        PC_LEAF_ASSIGN(result, parser(input));

        return guard.build(result);
    };
}

template<typename HeadP, typename... TailPs>
    requires AllSame<std::invoke_result_t<HeadP, StringRef &>, std::invoke_result_t<TailPs, StringRef &>...>
auto alt(HeadP p_head, TailPs... p_tail) {
    using ValueT = typename std::invoke_result_t<HeadP, StringRef &>::value_type;

    return [p_head, p_tail...](StringRef &input) -> ParseResult<ValueT> {
        ResultBuilder<ValueT> guard(input);
        auto result_with_error = p_head(input).or_else([&](auto) { return alt(p_tail...)(input); });
        PC_LEAF_ASSIGN(result, result_with_error);

        return guard.build(result);
    };
}

template<typename... Parsers>
    requires(sizeof...(Parsers) > 1 &&
             AllUnique<typename std::invoke_result_t<Parsers, StringRef &>::value_type...>)
auto alt(Parsers &&...parsers) {
    using ValueT = std::variant<typename std::invoke_result_t<Parsers, StringRef &>::value_type...>;

    auto wrap_parser = [](auto parser) {
        return [parser](StringRef& input) -> ParseResult<ValueT> {
            return parser(input).transform([](auto&& result) { return std::forward<decltype(result)>(result);});
        };
    };

    return alt(wrap_parser(parsers)...);
} }// namespace pc
