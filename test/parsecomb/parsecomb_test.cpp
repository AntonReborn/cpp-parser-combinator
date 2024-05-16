#include <gtest/gtest.h>
#include <vector>

#include "pc/parsecomb/sequence.h"
#include "pc/parsecomb/units.h"
#include "pc/parsecomb/multi.h"

template<typename T>
struct DebugLifetime {
    T value;
    std::string name;

    DebugLifetime(const T &val, const std::string &n) : value(val), name(n) {
        std::cout << "Constructed " << name << " with value: " << std::endl;
    }

    DebugLifetime(T &&val, const std::string &n) : value(std::move(val)), name(n) {
        std::cout << "Move constructed " << name << " with value: " << std::endl;
    }

    DebugLifetime(const DebugLifetime &other) : value(other.value), name(other.name) {
        std::cout << "Copy constructed " << name << " with value: " << std::endl;
    }

    DebugLifetime(DebugLifetime &&other) noexcept : value(std::move(other.value)), name(std::move(other.name)) {
        std::cout << "Move constructed (from DebugLifetime) " << name << " with value: " << std::endl;
    }

    DebugLifetime &operator=(const DebugLifetime &other) {
        if (this != &other) {
            value = other.value;
            name = other.name;
            std::cout << "Copy assigned (from DebugLifetime) " << name << " with value: " << std::endl;
        }
        return *this;
    }

    DebugLifetime &operator=(DebugLifetime &&other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
            name = std::move(other.name);
            std::cout << "Move assigned (from DebugLifetime) " << name << " with value: " << std::endl;
        }
        return *this;
    }

    ~DebugLifetime() {
        std::cout << "Destroyed " << name << " with value: " << std::endl;
    }

    T &get() { return value; }
    const T &get() const { return value; }
};

pc::ParseResult<pc::NothingT> separator(pc::StringRef &input) {
    PC_LEAF_ASSIGN(_, pc::tuple(pc::tag(","), pc::spaces(0))(input));

    return pc::Nothing;
}

using Coordinates = DebugLifetime<std::tuple<std::int32_t, std::int32_t, std::int32_t>>;
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

bool operator==(const Point &lhs, const Point &rhs) {
    return std::make_tuple(lhs.x, lhs.y, lhs.z) == std::make_tuple(rhs.x, rhs.y, rhs.z);
}

template<typename T>
bool operator== (const std::vector<T> &lhs, const std::vector<T> &rhs) {
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i])
            return false;
    }
    
    return true;
}

pc::ParseResult<Point> parse_point(pc::StringRef &input) {
    PC_LEAF_ASSIGN(_, pc::tag("Point: ")(input));
    PC_LEAF_ASSIGN(result, pc::discard_surround(
                                   pc::tag("("),
                                   parse_coordinates,
                                   pc::tag(")"))(input));

    auto [x, y, z] = result.get();
    return Point{x, y, z};
}

TEST(SimpleParsing, PointEasy) {
    std::string input = "Point: (1, -1100, 12);";
    pc::StringRef view(input);
    auto point = parse_point(view).value();
    auto expected = Point{1, -1100, 12};
    EXPECT_EQ(point, expected);
}

TEST(SimpleParsing, PointEasySpaces) {
    std::string input = "Point: (1,   -1100,-12)";
    pc::StringRef view(input);
    auto point = parse_point(view).value();
    auto expected = Point{1, -1100, -12};
    EXPECT_EQ(point, expected);
}

TEST(SimpleParsing, PointEasyVecto) {
    std::string input = "Point: (1,   -1100,-12); Point: (22, 33, 44); Point: (123, 12, -1);";
    pc::StringRef view(input);

    auto points_parser = pc::many_any(
       pc::discard_terminated(parse_point, pc::tuple(pc::tag(";"), pc::spaces(0))));
    auto point = points_parser(view).value();
    auto expected = std::vector<Point>{Point{1, -1100, -12}, Point {22, 33, 44}, Point {123, 12, -1}};

    EXPECT_EQ(point, expected);
    EXPECT_EQ(view.size(), 0);
}
