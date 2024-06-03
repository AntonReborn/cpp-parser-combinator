#pragma once

#include "pc/parsecomb/types.h"
#include "pc/parsecomb/units.h"
#include "pc/parsecomb/functional.h"
#include "sequence.h"

#include <expected>
#include <vector>

namespace pc {

template<typename T, typename F>
using Aggregator = std::tuple<T, F>;

// Add perfect forwarding
template<typename T>
using VectorAggregator = std::tuple<std::vector<T>, decltype([](auto& acc, auto&& x) { acc.push_back(x); })>;

using StringAggregator = std::tuple<std::string, decltype([](auto& acc, auto&& x) { acc.push_back(x); })>;

template<typename K, typename V>
using TupleToMapAggregator = std::tuple<std::map<K, V>, decltype([](auto& acc, auto&& x) { acc[std::get<0>(x)] = std::get<1>(x); })>;

template<typename P, typename T, typename F>
auto fold_many_lower_upper(size_t lower, size_t upper, P parser, Aggregator<T, F> aggregator) {
    return [lower, upper, parser, aggregator](StringRef &input) -> ParseResult<T> {
        ResultBuilder<T> guard(input);
        auto result = std::get<0>(aggregator);
        size_t i = 0;

        for (; i < upper; i++) {
            auto one_more = parser(input);
            if (one_more)
                std::get<1>(aggregator)(result, one_more.value());
            else
                break;
        }

        if (i < lower)
            return std::unexpected(ParseError::Unknown);

        return guard.build(std::move(result));
    };
}

template<typename P, typename T, typename F>
auto fold_many_any(P parser, const Aggregator<T, F>& aggregator) {
    return fold_many_lower_upper(0, pc::InfMany, parser, aggregator);
}

template<typename P, typename T, typename F>
auto fold_many_more(size_t lower, P parser, const Aggregator<T, F>& aggregator) {
    return fold_many_lower_upper(lower, pc::InfMany, parser, aggregator);
}

template<typename P>
auto many_lower_upper(size_t lower, size_t upper, P parser) {
    using ItemType = typename std::invoke_result_t<P, StringRef &>::value_type;

    return fold_many_lower_upper(lower, upper, parser, VectorAggregator<ItemType>());
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
auto fold_any_separated_by(P parser, S separator, Aggregator<T, F> aggregator) {
    return [parser, separator, aggregator](StringRef &input) -> ParseResult<T> {
        ResultBuilder<T> guard(input);

        PC_EXPECT_ASSIGN(prefix, fold_many_any(pc::take<0>(parser, separator), aggregator)(input));
        PC_EXPECT_ASSIGN(suffix, parser(input));

        std::get<F>(aggregator)(prefix, suffix);

        return guard.build(std::move(prefix));
    };
}

template<typename P, typename S>
auto many_any_separated_by(P parser, S separator) {
    using ItemType = typename std::invoke_result_t<P, StringRef &>::value_type;

    return fold_any_separated_by(parser, separator, VectorAggregator<ItemType>{});
}

}// namespace pc
