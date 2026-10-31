#include "AgentHarmony.hpp"
#include <ranges>
#include <type_traits>
#include "AgentHarmony.hpp"
#include <array>
#include <climits>
#include <limits>
#include <utility>
#include "AgentHarmony.hpp"
#include <gtest/gtest.h>
#include <array>

#include "../include/space/Field.hpp"

namespace test::field {
struct MyAgent {
    std::optional<agh::Point> pos;
    int value{};
};

TEST(FieldTest, MoveAgent) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.moveAgent(agent, {1, 0});
    EXPECT_EQ(*agent.pos, agh::Point(1,0));
    EXPECT_TRUE(field.isEmpty(agh::Point(1, 1)));
}

TEST(FieldTest, Apply) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.apply([](auto& a) { a.value += 2; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, Transform) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.transform([](agh::Point p, auto& a) { a.value += p.x + p.y; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, RemoveAgent) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    EXPECT_TRUE(field.getAgent({1,1}).has_value());
    field.removeAgent({1, 1});
    EXPECT_FALSE(field.getAgent({1,1}).has_value());
    field.addAgent(agent, {1, 1});
    EXPECT_TRUE(field.getAgent({1,1}).has_value());
    field.removeAgent(agent);
    EXPECT_FALSE(field.getAgent({1,1}).has_value());
}

TEST(FieldTest, GetEmpty) {
    using FieldT = agh::Field<MyAgent>;
    std::array<MyAgent, 4> agents{};
    FieldT field(2, 2);
    EXPECT_EQ(field.getEmpty().size(), 4);
    field.addAgent(agents[0], {1, 1});
    EXPECT_EQ(field.getEmpty().size(), 3);
    field.addAgent(agents[1], {1, 0});
    EXPECT_EQ(field.getEmpty().size(), 2);
    field.addAgent(agents[2], {0, 1});
    EXPECT_EQ(field.getEmpty().size(), 1);
    field.addAgent(agents[3], {0, 0});
    EXPECT_EQ(field.getEmpty().size(), 0);
}

TEST(FieldTest, ToToroidal) {
    using FieldT = agh::Field<MyAgent>;
    const FieldT field(2,2);

    agh::Point p = field.toToroidal({-2, -2});
    EXPECT_EQ(p.x, 0);
    EXPECT_EQ(p.y, 0);

    p = field.toToroidal({-3, -3});
    EXPECT_EQ(p.x, 1);
    EXPECT_EQ(p.y, 1);

    p = field.toToroidal({-4, -4});
    EXPECT_EQ(p.x, 0);
    EXPECT_EQ(p.y, 0);

    p = field.toToroidal({-5, -4});
    EXPECT_EQ(p.x, 1);
    EXPECT_EQ(p.y, 0);
}
}

namespace test::membership_field {
namespace {
struct Agent {
    std::optional<agh::Point> pos;
    bool operator==(const Agent&) const { return true; }
};
struct RealAgent {
    std::optional<agh::RealPoint> pos;
    bool operator==(const RealAgent&) const { return true; }
};
struct NoEquality { std::optional<agh::Point> pos; };
struct RealNoEquality { std::optional<agh::RealPoint> pos; };
}


TEST(MembershipTest, FieldRejectsDuplicatesAndForeignOperations) {
    agh::Field<Agent> field(2, 2), other(2, 2);
    Agent a, b, foreign;
    EXPECT_TRUE(field.addAgent(a, {0, 0}));
    EXPECT_TRUE(field.addAgent(b, {1, 0}));
    EXPECT_FALSE(field.addAgent(a, {0, 1}));
    EXPECT_FALSE(other.addAgent(a, {0, 1}));
    foreign.pos = agh::Point{0, 0};
    field.removeAgent(foreign);
    EXPECT_TRUE(foreign.pos);
    ASSERT_TRUE(field.getAgent({0, 0}));
    EXPECT_EQ(std::get<Agent*>(*field.getAgent({0, 0})), &a);
    EXPECT_THROW(field.moveAgent(foreign, {1, 1}), std::logic_error);
    EXPECT_TRUE(field.moveAgent(a, {0, 0}));
    EXPECT_FALSE(field.moveAgent(a, {1, 0}));
    EXPECT_EQ(*a.pos, (agh::Point{0, 0}));
    EXPECT_THROW(field.moveAgent(a, {2, 0}), std::out_of_range);
    EXPECT_TRUE(field.moveAgent(a, {1, 1}));
    field.removeAgent(a);
    field.removeAgent(a);
    EXPECT_FALSE(a.pos);
    EXPECT_TRUE(other.addAgent(a, {0, 0}));
}
}

namespace test::spatial_safety_field {
namespace {
struct GridAgent { std::optional<agh::Point> pos; int value{}; };
struct RealAgent { std::optional<agh::RealPoint> pos; int value{}; };
}


TEST(SpaceSafetyTest, RectangularEmptySetsField) {
    for (auto size : {agh::Point{1, 1}, agh::Point{4, 2}, agh::Point{2, 4}}) {
        agh::Field<GridAgent> single(size.x, size.y);
        std::vector<agh::Point> expected;
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x) expected.push_back({x, y});
        EXPECT_EQ(single.getEmpty(), expected);
        GridAgent a, b;
        single.addAgent(a, {0, 0});
        expected.erase(expected.begin());
        EXPECT_EQ(single.getEmpty(), expected);
    }
}

TEST(SpaceSafetyTest, RejectInvalidDimensionsAndUnrepresentableSizesField) {
    EXPECT_THROW(agh::Field<GridAgent>(0, 2), std::invalid_argument);
    EXPECT_THROW(agh::Field<GridAgent>(INT_MAX, INT_MAX), std::length_error);
}

TEST(SpaceSafetyTest, RejectInvalidCoordinatesWithoutChangingMembershipField) {
    agh::Field<GridAgent> field(2, 4);
    GridAgent a, b;
    EXPECT_THROW(field.addAgent(a, {2, 0}), std::out_of_range);
    EXPECT_FALSE(a.pos);
    EXPECT_THROW(std::as_const(field).getAgent({0, 4}), std::out_of_range);
    EXPECT_THROW((void)field.getNeighborhood({0, 0}, -1), std::invalid_argument);
}

TEST(SpaceSafetyTest, NormalizeDiscreteToroidalAccessField) {
    agh::Field<GridAgent> field(4, 2, true);
    GridAgent a, b;
    field.addAgent(a, {-1, -1});
    EXPECT_EQ(*a.pos, (agh::Point{3, 1}));
    EXPECT_FALSE(field.isEmpty({-1, -1}));
    EXPECT_EQ(field.toToroidal({INT_MIN, INT_MAX}), (agh::Point{0, 1}));
}
}

namespace test::api_field {
namespace {
struct A { std::optional<agh::Point> pos; int value{}; bool isActive() const { return true; } };
struct B { std::optional<agh::Point> pos; int value{}; };
struct R { std::optional<agh::RealPoint> pos; int value{}; };
struct S { std::optional<agh::RealPoint> pos; int value{}; };
struct OnlyA { void operator()(A&) const {} };
struct OnlyR { void operator()(R&) const {} };
struct EveryAgent { void operator()(auto&) const {} };
struct OnlyPointA { void operator()(agh::Point, A&) const {} };
struct EveryPointAgent { void operator()(agh::Point, auto&) const {} };
template<typename Space> constexpr bool acceptsPartial = requires(Space& space) { space.apply(OnlyA{}); };
template<typename Space, typename Visitor> constexpr bool acceptsVisitor =
    requires(Space& space, Visitor visitor) { space.apply(visitor); };
template<typename Space, typename Visitor> constexpr bool acceptsTransform =
    requires(Space& space, Visitor visitor) { space.transform(visitor); };
template<typename Space, typename Agent> constexpr bool acceptsAnyMembership =
    requires(Space& space, Agent& agent) { space.addAgent(agent, {}); } ||
    requires(Space& space, Agent& agent) { space.moveAgent(agent, {}); } ||
    requires(Space& space, Agent& agent) { space.removeAgent(agent); };
}


template<typename T> constexpr bool exposesClear = requires(T value) { value.clear(); };
static_assert(!acceptsPartial<agh::Field<A, B>>);
static_assert(acceptsVisitor<agh::Field<A, B>, EveryAgent>);
static_assert(acceptsTransform<agh::Field<A, B>, EveryPointAgent>);
static_assert(!acceptsTransform<agh::Field<A, B>, OnlyPointA>);
static_assert(acceptsAnyMembership<agh::Field<A>, A>);
static_assert(!acceptsAnyMembership<agh::Field<A>, B>);namespace {
struct PointOnlyGrid {
    bool outOfBounds(agh::Point) const;
    bool isToroidal() const;
    agh::Point toToroidal(agh::Point) const;
};
struct MutableOnlyGrid {
    bool outOfBounds(agh::Point);
    bool isToroidal();
    agh::Point toToroidal(agh::Point);
    int getWidth();
    int getHeight();
};
struct RvaluePointCallback {
    agh::Point operator()(agh::Point point) && { return point; }
};
struct RvalueAppendCallback {
    void operator()(agh::Point, std::vector<int>& values) && { values.push_back(1); }
};
}


TEST(ApiContractTest, StructuralAccessorsAreReadOnlyField) {
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::declval<agh::Field<A>&>().getAgent({0, 0}))>>));
    EXPECT_FALSE((acceptsPartial<agh::Field<A, B>>));
}

TEST(ApiContractTest, FieldTraversalRejectsStructuralMutationsAndRecovers) {
    agh::Field<A, B> field(2, 2);
    A a, extra;
    B b;
    field.addAgent(a, {0, 0});
    field.addAgent(b, {1, 0});
    EXPECT_THROW(field.apply([&](auto&) { field.removeAgent(a); }), std::logic_error);
    EXPECT_THROW(field.transform([&](agh::Point, auto&) { field.addAgent(extra, {0, 1}); }), std::logic_error);
    EXPECT_THROW(field.apply([&](auto&) { field.removeAgent(agh::Point{0, 0}); }), std::logic_error);
    field.apply([&](auto&) { field.apply([](auto& agent) { ++agent.value; }); });
    EXPECT_EQ(a.value, 2);
    EXPECT_EQ(b.value, 2);
    EXPECT_TRUE(field.moveAgent(a, {1, 1}));
}

TEST(ApiContractTest, GridConceptRequiresConstDimensionsAndAccess) {
    EXPECT_FALSE(agh::Grid<PointOnlyGrid>);
    EXPECT_FALSE(agh::Grid<MutableOnlyGrid>);
    EXPECT_TRUE((agh::Grid<agh::Field<A>>));
}

TEST(ApiContractTest, NeighborhoodUtilitiesPreserveRvalueQualifiedCallbacks) {
    agh::Field<A> field(3, 3);
    auto points = agh::visitNeighborhood(field, {1, 1}, 1, true, true, RvaluePointCallback{});
    auto values = agh::visitNeighbors<agh::Field<A>, int>(field, {1, 1}, 1, true, true, RvalueAppendCallback{});
    EXPECT_EQ(points.size(), 9);
    EXPECT_EQ(values.size(), 9);
}
}
