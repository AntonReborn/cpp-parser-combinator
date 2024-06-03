#pragma once

#include "pc/parsecomb/types.h"
#include "pc/parsecomb/traits.h"
#include <tuple>
#include <type_traits>

namespace pc {

template<typename Parser, typename F>
auto map(Parser p, F f) {
    return [p, f](StringRef& input) -> ParseResult<std::invoke_result_t<F, typename std::invoke_result_t<Parser, StringRef&>::value_type>> {
        PC_EXPECT_ASSIGN(result, p(input));

        return f(result);
    };
}

template<typename Parser, typename EmitT>
auto to(Parser p, EmitT emit) {
    return map(p, [emit](const auto&) { return emit; });
}

}