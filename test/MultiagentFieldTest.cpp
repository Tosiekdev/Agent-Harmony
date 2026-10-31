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

#include "../include/space/MultiagentField.hpp"

namespace test::multiagent_field {
struct MyAgent {
    int id{};
    std::optional<agh::Point> pos;
    int value{};

    bool operator==(const MyAgent& rhs) const {
        return id == rhs.id;
    }
};

TEST(MultiagentFieldTest, AddAgent) {
    using Field = agh::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    EXPECT_EQ(field.getAgents({0,0}).size(), 2);
}

TEST(MultiagentFieldTest, Apply) {
    using Field = agh::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.apply([](auto& a) { a.value += 2; });
    EXPECT_EQ(agent.value, 2);
    EXPECT_EQ(agent2.value, 2);
}

TEST(MultiagentFieldTest, Transform) {
    using Field = agh::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    MyAgent agent3{3};
    Field field(2, 2);
    field.addAgent(agent, {0, 1});
    field.addAgent(agent2, {1, 1});
    field.addAgent(agent3, {1, 1});
    field.transform([](agh::Point p, auto& a) {
        a.value += p.x + p.y;
    });
    EXPECT_EQ(agent.value, 1);
    EXPECT_EQ(agent2.value, 2);
    EXPECT_EQ(agent3.value, 2);
}

TEST(MultiagentFieldTest, RemoveAgent) {
    using Field = agh::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.removeAgent(agent);
    EXPECT_EQ(field.getAgents({0,0}).size(), 1);
    EXPECT_FALSE(agent.pos.has_value());
}

TEST(MultiagentFieldTest, RemoveAgents) {
    using Field = agh::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.removeAgents({0, 0});
    EXPECT_EQ(field.getAgents({0,0}).size(), 0);
    EXPECT_FALSE(agent.pos.has_value());
    EXPECT_FALSE(agent2.pos.has_value());
}

TEST(MultiagentFieldTest, GetNeighborhood) {
    using Field = agh::MultiagentField<MyAgent>;
    Field field(3, 3);
    auto moore = field.getNeighborhood({1, 1}, 1, true, false);
    auto mooreCenter = field.getNeighborhood({1, 1}, 1, true, true);

    auto vonNeumann = field.getNeighborhood({1, 1}, 1, false, false);
    auto vonNeumannCenter = field.getNeighborhood({1, 1}, 1, false, true);

    EXPECT_EQ(moore.size(), 8);
    EXPECT_EQ(mooreCenter.size(), 9);
    EXPECT_EQ(vonNeumann.size(), 4);
    EXPECT_EQ(vonNeumannCenter.size(), 5);
}

namespace ngh {
using Field = agh::MultiagentField<MyAgent>;
class NeighborhoodTest : public ::testing::Test {
protected:
std::array<MyAgent, 9> agents{};
Field field{3, 3};

void SetUp() override {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            field.addAgent(agents[y * 3 + x], {x, y});
            agents[y * 3 + x].id = y * 3 + x;
        }
    }
}

};

TEST_F(NeighborhoodTest, GetMooreNeighbors) {
    auto moore = field.getNeighbors({1, 1}, 1, true, false);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(moore.size(), 8);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            const auto index = y * 3 + x;
            if (index < 4) {
                EXPECT_TRUE(equalityCheck(moore[index], agents[index]));
            }
            else if (index > 4) {
                EXPECT_TRUE(equalityCheck(moore[index-1], agents[index]));
            }
        }
    }
}

TEST_F(NeighborhoodTest, GetMooreNeighborsNoCenter) {
    auto mooreCenter = field.getNeighbors({1, 1}, 1, true, true);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(mooreCenter.size(), 9);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            const auto index = y * 3 + x;
            EXPECT_TRUE(equalityCheck(mooreCenter[index], agents[index]));
        }
    }
}

TEST_F(NeighborhoodTest, GetVonNeumannNeighbors) {
    auto vonNeumann = field.getNeighbors({1, 1}, 1, false, false);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(vonNeumann.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(equalityCheck(vonNeumann[i], agents[2*i+1]));
    }
}

TEST_F(NeighborhoodTest, GetVonNeumannNeighborsNoCenter) {
    auto vonNeumannCenter = field.getNeighbors({1, 1}, 1, false, true);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(vonNeumannCenter.size(), 5);
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[0], agents[1]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[1], agents[3]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[2], agents[4]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[3], agents[5]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[4], agents[7]));
}
}

TEST(MultiagentFieldTest, GetEmpty) {
    using FieldT = agh::MultiagentField<MyAgent>;
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
}

namespace test::membership_multiagent {
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


TEST(MembershipTest, MultiagentRemovalUsesIdentity) {
    agh::MultiagentField<Agent> field(2, 2), other(2, 2);
    Agent a, b, foreign;
    field.addAgent(a, {0, 0});
    field.addAgent(b, {0, 0});
    EXPECT_THROW(field.addAgent(a, {1, 0}), std::logic_error);
    EXPECT_THROW(other.addAgent(a, {1, 0}), std::logic_error);
    foreign.pos = agh::Point{0, 0};
    field.removeAgent(foreign);
    EXPECT_TRUE(foreign.pos);
    EXPECT_THROW(field.moveAgent(foreign, {1, 0}), std::logic_error);
    EXPECT_THROW(field.moveAgent(a, {2, 0}), std::out_of_range);
    EXPECT_EQ(field.agentCount({0, 0}), 2);
    field.moveAgent(a, {0, 0});
    field.removeAgent(a);
    ASSERT_EQ(field.getAgents({0, 0}).size(), 1);
    EXPECT_EQ(std::get<Agent*>(field.getAgents({0, 0})[0]), &b);
    EXPECT_TRUE(b.pos);
    EXPECT_FALSE(a.pos);
    other.addAgent(a, {1, 1});
    field.moveAgent(b, {1, 0});
    EXPECT_TRUE(field.isEmpty({0, 0}));
}

TEST(MembershipTest, MovingAndRemovalDoNotRequireValueEqualityMultiagent) {
    agh::MultiagentField<NoEquality> grid(2, 2);
    NoEquality a;
    grid.moveAgent(a, {0, 0});
    grid.moveAgent(a, {1, 1});
    grid.removeAgent(a);
    EXPECT_FALSE(a.pos);

}
}

namespace test::spatial_safety_multi {
namespace {
struct GridAgent { std::optional<agh::Point> pos; int value{}; };
struct RealAgent { std::optional<agh::RealPoint> pos; int value{}; };
}


TEST(SpaceSafetyTest, RectangularEmptySetsMultiagentField) {
    for (auto size : {agh::Point{1, 1}, agh::Point{4, 2}, agh::Point{2, 4}}) {
        agh::MultiagentField<GridAgent> multi(size.x, size.y);
        std::vector<agh::Point> expected;
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x) expected.push_back({x, y});
        EXPECT_EQ(multi.getEmpty(), expected);
        GridAgent a, b;
        multi.addAgent(b, {0, 0});
        expected.erase(expected.begin());
        EXPECT_EQ(multi.getEmpty(), expected);
    }
}

TEST(SpaceSafetyTest, RejectInvalidDimensionsAndUnrepresentableSizesMultiagentField) {
    EXPECT_THROW(agh::MultiagentField<GridAgent>(2, -1), std::invalid_argument);
}

TEST(SpaceSafetyTest, RejectInvalidCoordinatesWithoutChangingMembershipMultiagentField) {
    agh::MultiagentField<GridAgent> multi(2, 4);
    GridAgent a, b;
    EXPECT_THROW(multi.addAgent(b, {-1, 0}), std::out_of_range);
    EXPECT_FALSE(b.pos);
    EXPECT_THROW(std::as_const(multi).getAgents({2, 0}), std::out_of_range);
    EXPECT_THROW((void)multi.getNeighbors({0, 0}, -1, true, false), std::invalid_argument);
}

TEST(SpaceSafetyTest, NormalizeDiscreteToroidalAccessMultiagentField) {
    agh::MultiagentField<GridAgent> multi(4, 2, true);
    GridAgent a, b;
    multi.addAgent(b, {4, 2});
    EXPECT_EQ(*b.pos, (agh::Point{0, 0}));
    EXPECT_EQ(multi.getAgents({4, 2}).size(), 1);
}
}

namespace test::api_multiagentfield {
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
static_assert(!acceptsPartial<agh::MultiagentField<A, B>>);
static_assert(acceptsVisitor<agh::MultiagentField<A, B>, EveryAgent>);
static_assert(acceptsTransform<agh::MultiagentField<A, B>, EveryPointAgent>);
static_assert(!acceptsTransform<agh::MultiagentField<A, B>, OnlyPointA>);
static_assert(acceptsAnyMembership<agh::MultiagentField<A>, A>);
static_assert(!acceptsAnyMembership<agh::MultiagentField<A>, B>);
TEST(ApiContractTest, StructuralAccessorsAreReadOnlyMultiagentField) {
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::declval<agh::MultiagentField<A>&>().getAgents({0, 0}))>>));
    EXPECT_FALSE((acceptsPartial<agh::MultiagentField<A, B>>));
}

TEST(ApiContractTest, MultiagentTraversalRejectStructuralMutations) {
    agh::MultiagentField<A, B> grid(2, 2);
    A a, extra;
    grid.addAgent(a, {0, 0});
    EXPECT_THROW(grid.apply([&](auto&) { grid.removeAgent(a); }), std::logic_error);
    EXPECT_THROW(grid.transform([&](agh::Point, auto&) { grid.removeAgents({0, 0}); }), std::logic_error);
    EXPECT_THROW(grid.apply([&](auto&) { grid.moveAgent(a, {1, 1}); }), std::logic_error);
    EXPECT_THROW(grid.apply([&](auto&) { grid.addAgent(extra, {1, 1}); }), std::logic_error);
    grid.removeAgent(a);
    EXPECT_FALSE(a.pos);

}
}
