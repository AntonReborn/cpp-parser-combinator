#include <gtest/gtest.h>
#include <variant>
#include <vector>

#include "pc/parsecomb/branch.h"
#include "pc/parsecomb/multi.h"
#include "pc/parsecomb/sequence.h"
#include "pc/parsecomb/types.h"
#include "pc/parsecomb/units.h"

template<typename T, bool Enabled = true>
struct DebugLifetime {
    T value;
    std::string name;

    DebugLifetime(const T &val, const std::string &n) : value(val), name(n) {
        if (!Enabled) return;
        std::cout << "Constructed " << name << " with value: " << std::endl;
    }

    DebugLifetime(T &&val, const std::string &n) : value(std::move(val)), name(n) {
        if (!Enabled) return;
        std::cout << "Move constructed " << name << " with value: " << std::endl;
    }

    DebugLifetime(const DebugLifetime &other) : value(other.value), name(other.name) {
        if (!Enabled) return;
        std::cout << "Copy constructed " << name << " with value: " << std::endl;
    }

    DebugLifetime(DebugLifetime &&other) noexcept : value(std::move(other.value)), name(std::move(other.name)) {
        if (!Enabled) return;
        std::cout << "Move constructed (from DebugLifetime) " << name << " with value: " << std::endl;
    }

    DebugLifetime &operator=(const DebugLifetime &other) {
        if (this != &other) {
            value = other.value;
            name = other.name;
            if (!Enabled) return *this;
            std::cout << "Copy assigned (from DebugLifetime) " << name << " with value: " << std::endl;
        }
        return *this;
    }

    DebugLifetime &operator=(DebugLifetime &&other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
            name = std::move(other.name);
            if (!Enabled) return;
            std::cout << "Move assigned (from DebugLifetime) " << name << " with value: " << std::endl;
        }
        return *this;
    }

    ~DebugLifetime() {
        if (!Enabled) return;
        std::cout << "Destroyed " << name << " with value: " << std::endl;
    }

    T &get() { return value; }
    const T &get() const { return value; }
};

pc::ParseResult<pc::NothingT> separator(pc::StringRef &input) {
    PC_EXPECT_ASSIGN(_, pc::tuple(pc::tag(","), pc::spaces(0))(input));

    return pc::Nothing;
}

using Coordinates = DebugLifetime<std::tuple<std::int32_t, std::int32_t, std::int32_t>, false>;
pc::ParseResult<Coordinates> parse_coordinates(pc::StringRef &input) {
    PC_EXPECT_ASSIGN(result, pc::tuple(pc::int32(),
                                     separator,
                                     pc::int32(),
                                     separator,
                                     pc::int32())(input));

    auto [x, _, y, __, z] = result;
    return DebugLifetime<std::tuple<std::int32_t, std::int32_t, std::int32_t>, false>(std::make_tuple(x, y, z), "{coordinate}");
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
bool operator==(const std::vector<T> &lhs, const std::vector<T> &rhs) {
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i])
            return false;
    }

    return true;
}

pc::ParseResult<Point> parse_point(pc::StringRef &input) {
    PC_EXPECT_ASSIGN(_, pc::tag("Point: ")(input));
    PC_EXPECT_ASSIGN(result, pc::discard_surround(
                                   pc::tag("("),
                                   parse_coordinates,
                                   pc::tag(")"))(input));

    auto [x, y, z] = result.get();
    return Point{x, y, z};
}

pc::ParseResult<Point> parse_zero_point(pc::StringRef &input) {
    PC_EXPECT_ASSIGN(_, pc::tag("ZeroPoint")(input));

    return Point{0, 0, 0};
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

TEST(SimpleParsing, PointEasyVector) {
    std::string input = "Point: (1,   -1100,-12); Point: (22, 33, 44); Point: (123, 12, -1);";
    pc::StringRef view(input);

    auto points_parser = pc::many_any(
            pc::discard_terminated(parse_point, pc::tuple(pc::tag(";"), pc::spaces(0))));
    auto point = points_parser(view).value();
    auto expected = std::vector<Point>{Point{1, -1100, -12}, Point{22, 33, 44}, Point{123, 12, -1}};

    EXPECT_EQ(point, expected);
    EXPECT_EQ(view.size(), 0);
}

TEST(SimpleParsing, PointEasyAltSameType) {
    std::string input = "Point: (1,   -1100,-12);ZeroPoint";
    {
        pc::StringRef view(input);

        auto points_or_circle = pc::alt(parse_point, parse_zero_point);
        auto result1 = points_or_circle(view).value();
        pc::tag(";")(view);
        auto result2 = points_or_circle(view).value();

        EXPECT_EQ(view.size(), 0);
        EXPECT_EQ(result1.x, 1);
        EXPECT_EQ(result1.z, -12);
        EXPECT_EQ(result2.x, 0);
        EXPECT_EQ(result2.z, 0);
    }
}

struct Circle {
    std::int32_t radius;
};

pc::ParseResult<Circle> parse_circle(pc::StringRef &input) {
    PC_EXPECT_ASSIGN(_, pc::tag("Circle: ")(input));
    PC_EXPECT_ASSIGN(radius, pc::int32()(input));
    return Circle{radius};
}

TEST(SimpleParsing, PointEasyAltDifferentType) {
    std::string input = "Point: (1,   -1100,-12); Circle: 123";
    {
        pc::StringRef view(input);
        auto points_or_circle = pc::alt(parse_point, parse_circle);
        auto result1 = points_or_circle(view).value();
        pc::tag("; ")(view);
        auto result2 = points_or_circle(view).value();

        EXPECT_TRUE(std::holds_alternative<Point>(result1));
        EXPECT_TRUE(std::holds_alternative<Circle>(result2));
    }
}
