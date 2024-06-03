# C++ Parser Combinator
Parser combinator library for C++.

### Json Parser example
Json parser example follows precisely high-level parts of grammar from [json.org](http://www.json.org). 

#### In memory data types
```c++
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
```
#### Leaf element parsers (simplified)
```c++
const auto ws = pc::chr(' ') | pc::chr('\n') | pc::chr('\t');
const auto wss = many_any(ws);
const auto character = ws | pc::char_range('a', 'z') | pc::char_range('A', 'Z') | pc::char_range('0', '9');
const auto number = pc::int32();
const auto string = take<1>(pc::chr('"'), fold_many_any(character, pc::StringAggregator{}), pc::chr('"'));

pc::ParseResult<Object> object(pc::StringRef &input);
pc::ParseResult<Array> array(pc::StringRef &input);

const auto value = alt(string, number, object, array);
```

#### Array parser
```c++
/**
 * From `json.org`:
 * array
 *    '[' ws ']'
 *    '[' elements ']'
 *
 * elements
 *    element
 *    element ',' elements
 *
 * element
 *    ws value ws
 */
const auto element = map(take<1>(wss, value, wss), to_value);
const auto elements = many_any_separated_by(element, pc::chr(','));

const auto empty_array = to(take<1>(pc::chr('['), wss, pc::chr(']')), Array{});
const auto non_empty_array = take<1>(pc::chr('['), elements, pc::chr(']'));

pc::ParseResult<Array> array(pc::StringRef &input) {
    return alt(empty_array, non_empty_array)(input);
}
```
### Object parser
```c++
/**
 * From `json.org`:
 * object
 *     '{' ws '}'
 *     '{' members '}'
 *
 * members
 *     member
 *     member ',' members
 *
 * member
 *     ws string ws ':' element
 */
const auto member = pc::take<0, 2>(pc::take<1>(wss, string, wss), pc::chr(':'), element);
const auto members = fold_any_separated_by(member, pc::chr(','), pc::TupleToMapAggregator<std::string, Value>{});

const auto empty_object = to(take<1>(pc::chr('{'), wss, pc::chr('}')), Object{});
const auto non_empty_object = take<1>(pc::chr('{'), members, pc::chr('}'));

pc::ParseResult<Object> object(pc::StringRef &input) {
    return alt(empty_object, non_empty_object)(input);
}

const auto json = element;
```
#### Usage example
```c++
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
```
