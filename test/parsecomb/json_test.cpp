#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "pc/parsecomb/branch.h"
#include "pc/parsecomb/multi.h"
#include "pc/parsecomb/sequence.h"
#include "pc/parsecomb/traits.h"
#include "pc/parsecomb/types.h"
#include "pc/parsecomb/units.h"

#include "utils/jsutils.h"

namespace js {
struct Value;

using String = std::string;
using Char = char;
using Number = std::int32_t;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value>;

using Variant = std::variant<
        String,
        Number,
        Object,
        Array>;

struct Value : public Variant {
    using variant::variant;
};

Value to_value(const Variant &var) {
    return std::visit([](auto &&arg) -> Value { return arg; }, var);
}

using Json = Value;

namespace parser {
const auto ws = pc::char_exact(' ') | pc::char_exact('\n') | pc::char_exact('\t');
const auto wss = many_any(ws);
const auto character = ws | pc::char_range('a', 'z') | pc::char_range('A', 'Z') | pc::char_range('0', '9');
const auto number = pc::int32();
const auto string = discard_surround(
        pc::tag("\""),
        fold_many_any(character, std::string{},
                      [](auto &accum, const auto &item) { accum.push_back(item); }),
        pc::tag("\""));

pc::ParseResult<Object> object(pc::StringRef &input);
pc::ParseResult<Array> array(pc::StringRef &input);

const auto value = alt(string,
                       number,
                       object,
                       array);
// ---- Array ----
const auto element = map(discard_surround(wss, value, wss), to_value);
const auto elements = many_any_separated_by(element, pc::tag(","));

const auto empty_array = to(discard_surround(pc::tag("["), wss, pc::tag("]")), Array{});
const auto non_empty_array = discard_surround(pc::tag("["), elements, pc::tag("]"));

pc::ParseResult<Array> array(pc::StringRef &input) {
    return alt(empty_array, non_empty_array)(input);
}

// ---- Object ----
const auto member = tuple(discard_surround(wss, string, wss), pc::tag(":"), element);
const auto members = fold_any_separated_by(member, pc::tag(","), std::map<std::string, Value>{},
                                           [](auto &accumulator, auto new_member) {
                                               accumulator[std::get<0>(new_member)] = std::get<2>(new_member);
                                           });

const auto empty_object = to(discard_surround(pc::tag("{"), wss, pc::tag("}")), Object{});
const auto non_empty_object = discard_surround(pc::tag("{"), members, pc::tag("}"));

pc::ParseResult<Object> object(pc::StringRef &input) {
    return alt(empty_object, non_empty_object)(input);
}

const auto json = element;
}// namespace parser
}// namespace js


TEST(JsonParser, BaseJsonParser) {
    std::string input = R"({
    "Empire State Building": {
        "height": 1250,
        "floors": 102,
        "meta": [1, 13, -223, "unknown"]
    }
})";
    pc::StringRef view(input);
    auto result = js::parser::json(view);

    EXPECT_TRUE(result.has_value());

    auto are_equal = js::equal(result.value(),
                               js::Object{{"Empire State Building", js::Object{
                                                                            {"height", js::Number{1250}},
                                                                            {"floors", js::Number{102}},
                                                                            {"meta", js::Array{js::Number{1}, js::Number{13}, js::Number{-223}, js::String{"unknown"}}},
                                                                    }}});

    EXPECT_TRUE(are_equal);
}
