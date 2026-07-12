#include <gtest/gtest.h>
#include "draft/util/reflectable.hpp"
#include <string>
#include <tuple>
#include <vector>

namespace {
    struct Point {
        DRAFT_REFLECTED(int, x) = 0;
        DRAFT_REFLECTED(int, y) = 0;

        DRAFT_REFLECTABLE(Point, x, y)
    };

    struct Mixed {
        DRAFT_REFLECTED(int, count) = 0;
        DRAFT_REFLECTED(std::string, name);

        DRAFT_REFLECTABLE(Mixed, count, name)
    };

    struct Empty {
        DRAFT_REFLECTABLE(Empty)
    };

    struct SingleField {
        DRAFT_REFLECTED(int, only) = 0;

        DRAFT_REFLECTABLE(SingleField, only)
    };

    struct ManyFields {
        DRAFT_REFLECTED(int, a) = 1;
        DRAFT_REFLECTED(int, b) = 2;
        DRAFT_REFLECTED(int, c) = 3;
        DRAFT_REFLECTED(int, d) = 4;
        DRAFT_REFLECTED(int, e) = 5;
        DRAFT_REFLECTED(int, f) = 6;
        DRAFT_REFLECTED(int, g) = 7;

        DRAFT_REFLECTABLE(ManyFields, a, b, c, d, e, f, g)
    };
}

TEST(Reflectable, ReflectIsConstexprAndHasExpectedSize)
{
    constexpr auto fields = Point::reflect();
    static_assert(std::tuple_size_v<decltype(fields)> == 2);
}

TEST(Reflectable, SatisfiesReflectableConcept)
{
    static_assert(Draft::Reflectable<Point>);
    static_assert(Draft::Reflectable<Mixed>);
    static_assert(Draft::Reflectable<Empty>);
    static_assert(!Draft::Reflectable<int>);
}

TEST(Reflectable, FieldsCaptureNamesAndValues)
{
    Point p{3, 4};
    auto fields = Point::reflect();

    ASSERT_EQ(std::get<0>(fields).name, "x");
    ASSERT_EQ(std::get<0>(fields).get(p), 3);

    ASSERT_EQ(std::get<1>(fields).name, "y");
    ASSERT_EQ(std::get<1>(fields).get(p), 4);
}

TEST(Reflectable, FieldSetMutatesTheInstance)
{
    Point p{3, 4};
    auto fields = Point::reflect();

    std::get<0>(fields).set(p, 10);
    ASSERT_EQ(p.x, 10);
    ASSERT_EQ(p.y, 4);
}

TEST(Reflectable, ForEachFieldVisitsEveryMemberWithItsName)
{
    Point p{5, 6};
    std::vector<std::string> names;
    int sum = 0;

    Draft::for_each_field(p, [&](std::string_view name, auto& value){
        names.emplace_back(name);
        sum += value;
    });

    ASSERT_EQ(names, (std::vector<std::string>{"x", "y"}));
    ASSERT_EQ(sum, 11);
}

TEST(Reflectable, ForEachFieldWorksOnConstInstances)
{
    const Point p{5, 6};
    int sum = 0;

    Draft::for_each_field(p, [&](std::string_view, const auto& value){
        sum += value;
    });

    ASSERT_EQ(sum, 11);
}

TEST(Reflectable, SupportsHeterogeneousMemberTypes)
{
    Mixed m;
    m.count = 2;
    m.name = "widgets";

    auto fields = Mixed::reflect();
    ASSERT_EQ(std::get<0>(fields).get(m), 2);
    ASSERT_EQ(std::get<1>(fields).get(m), "widgets");
}

TEST(Reflectable, SupportsZeroFields)
{
    static_assert(std::tuple_size_v<decltype(Empty::reflect())> == 0);
}

TEST(Reflectable, SupportsExactlyOneField)
{
    SingleField s{42};
    auto fields = SingleField::reflect();

    static_assert(std::tuple_size_v<decltype(fields)> == 1);
    ASSERT_EQ(std::get<0>(fields).name, "only");
    ASSERT_EQ(std::get<0>(fields).get(s), 42);
}

TEST(Reflectable, ScalesPastTheFourFieldExpansionGranularity)
{
    ManyFields m;
    static_assert(std::tuple_size_v<decltype(ManyFields::reflect())> == 7);

    std::vector<std::string> names;
    int sum = 0;

    Draft::for_each_field(m, [&](std::string_view name, auto& value){
        names.emplace_back(name);
        sum += value;
    });

    ASSERT_EQ(names, (std::vector<std::string>{"a", "b", "c", "d", "e", "f", "g"}));
    ASSERT_EQ(sum, 1 + 2 + 3 + 4 + 5 + 6 + 7);
}

TEST(Reflectable, VisitFieldFindsByNameAndReportsWhetherFound)
{
    Point p{7, 8};
    int seen = -1;

    bool foundY = Draft::visit_field(p, "y", [&](auto& value){ seen = value; });
    ASSERT_TRUE(foundY);
    ASSERT_EQ(seen, 8);

    bool foundZ = Draft::visit_field(p, "z", [&](auto&){ FAIL() << "should not be called"; });
    ASSERT_FALSE(foundZ);
}

TEST(Reflectable, VisitFieldCanMutateThroughTheVisitor)
{
    Point p{7, 8};
    Draft::visit_field(p, "x", [](auto& value){ value = 100; });
    ASSERT_EQ(p.x, 100);
}
