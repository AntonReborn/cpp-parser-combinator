# C++ Parser Combinator
cpp parser combinator

### Start new project
```c++
pc::ParseResult<pc::NothingT> separator(pc::StringRef &input) {
    PC_LEAF_ASSIGN(_, pc::tuple(pc::tag(","), pc::spaces(0))(input));

    return pc::Nothing;
}

using Coordinates = std::tuple<std::int32_t, std::int32_t, std::int32_t>;
pc::ParseResult<Coordinates> parse_coordinates(pc::StringRef &input) {
    PC_LEAF_ASSIGN(result, pc::tuple(pc::int32(),
                                     separator,
                                     pc::int32(),
                                     separator,
                                     pc::int32())(input));

    auto [x, _, y, __, z] = result;
    return DebugLifetime(std::make_tuple(x, y, z), "{coordinate}");
}

struct Point {
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
};

pc::ParseResult<Point> parse_point(pc::StringRef &input) {
    PC_LEAF_ASSIGN(_, pc::tag("Point: ")(input));
    PC_LEAF_ASSIGN(result, pc::delimeted(
                                   pc::tag("("),
                                   parse_coordinates,
                                   pc::tag(")"))(input));

    auto [x, y, z] = result.get();
    return Point{x, y, z};
}

// parse_point("Point: (12,  -1, 1000)").value() == Point {12, -1, 1000}
```
