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
#include <random>
#include <set>
#include "space/ContinuousSpace.hpp"
#include <gtest/gtest.h>

#include "../include/space/ContinuousSpace.hpp"

namespace test::continuous {
struct Agent {
    static int idCount;
    int id;
    std::optional<agh::RealPoint> pos;
    Agent() {
        id = idCount;
        ++idCount;
    }
    bool operator==(const Agent& rhs) const {
        return id == rhs.id;
    }
};
int Agent::idCount = 0;

TEST(ContinuousSpaceTest, AddAgent) {
    agh::ContinuousSpace<Agent> space(10.f, 10.f, 1.f);
    Agent a, b, c;

    space.addAgent(a, {5.f, 5.f});
    space.addAgent(b, {3.f, 3.f});
    space.addAgent(c, {2.f, 2.f});

    EXPECT_EQ(space.agentCount({5.f, 5.f}), 1);
    EXPECT_EQ(space.agentCount({3.f, 3.f}), 1);
    EXPECT_EQ(space.agentCount({2.f, 2.f}), 1);
}

TEST(ContinuousSpaceTest, GetNeighbours) {
    agh::ContinuousSpace<Agent> space(10.f, 10.f, 1.f);
    Agent a, b, c;

    space.addAgent(a, {5.f, 5.f});
    space.addAgent(b, {3.f, 3.f});
    space.addAgent(c, {2.f, 2.f});

    auto nghs = space.getNeighbors(*a.pos, 3.f, true, false);
    EXPECT_EQ(nghs.size(), 1);

    nghs = space.getNeighbors(*a.pos, 3.f, false, true);
    EXPECT_EQ(nghs.size(), 3);
}
}

namespace test::continuous_geometry {

namespace {
struct Agent { std::optional<agh::RealPoint> pos; };
}

TEST(ContinuousGeometryTest, OwnBucketBoundariesAndPeriodicSeam) {
    agh::ContinuousSpace<Agent> space(4, 2, 1, true);
    Agent a, b, c;
    space.addAgent(a, {0.5f, 0.5f});
    space.addAgent(b, {1.05f, 1});
    space.addAgent(c, {3.9f, 1});
    EXPECT_EQ(space.getNeighbors({0.5f, 0.5f}, 0, true, true).size(), 1);
    EXPECT_TRUE(space.getNeighbors({0.5f, 0.5f}, 0, false, false).empty());
    EXPECT_EQ(space.getNeighbors({0.95f, 1}, 0.2f).size(), 1);
    EXPECT_EQ(space.getNeighbors({0.1f, 1}, 0.25f).size(), 1);
    EXPECT_EQ(space.getNeighbors({-3.9f, 1}, 0.25f).size(), 1);
    EXPECT_EQ(space.getNeighbors({0, 0}, std::numeric_limits<float>::max()).size(), 3);
}

TEST(ContinuousGeometryTest, MatchesIndependentFullScanAndPeriodicImages) {
    std::mt19937 random(4091);
    for (auto extent : {agh::RealPoint{4.3f, 2.2f}, agh::RealPoint{2, 4}})
    for (bool torus : {false, true}) {
        agh::ContinuousSpace<Agent> space(extent.x, extent.y, 0.7f, torus);
        std::array<Agent, 40> agents{};
        std::uniform_real_distribution<float> x(0, std::nextafter(extent.x, 0.0f));
        std::uniform_real_distribution<float> y(0, std::nextafter(extent.y, 0.0f));
        for (auto& agent : agents) space.addAgent(agent, {x(random), y(random)});
        space.moveAgent(agents[0], {0, 0});
        space.moveAgent(agents[1], {0, 0});
        space.moveAgent(agents[2], {std::nextafter(extent.x, 0.0f), std::nextafter(extent.y, 0.0f)});
        for (int query = 0; query < 40; ++query) {
            const agh::RealPoint origin = query == 0 ? agh::RealPoint{0, 0} : agh::RealPoint{x(random), y(random)};
            for (float radius : {0.0f, 0.01f, 0.2f, 0.7f, 1.5f, 20.0f})
            for (bool euclidean : {false, true}) for (bool center : {false, true}) {
                std::set<Agent*> expected, actual;
                for (auto& agent : agents) {
                    if (!center && *agent.pos == origin) continue;
                    double distance = std::numeric_limits<double>::infinity();
                    const int images = torus ? 1 : 0;
                    for (int ix = -images; ix <= images; ++ix) for (int iy = -images; iy <= images; ++iy) {
                        const double dx = double(agent.pos->x) + double(ix) * extent.x - origin.x;
                        const double dy = double(agent.pos->y) + double(iy) * extent.y - origin.y;
                        distance = std::min(distance, euclidean ? std::sqrt(dx * dx + dy * dy) : std::max(std::abs(dx), std::abs(dy)));
                    }
                    if (distance <= radius) expected.insert(&agent);
                }
                auto neighbors = space.getNeighbors(origin, radius, euclidean, center);
                for (auto entry : neighbors) actual.insert(std::get<Agent*>(entry));
                ASSERT_EQ(neighbors.size(), actual.size());
                ASSERT_EQ(actual, expected) << "torus=" << torus << " r=" << radius << " euclidean=" << euclidean;
            }
        }
    }
}

TEST(ContinuousGeometryTest, EuclideanQueryRejectsAgentsOutsideCircleInsideCandidateBuckets) {
    for (bool torus : {false, true}) {
        agh::ContinuousSpace<Agent> space(40, 40, 10, torus);
        Agent diagonal, outsideCircle, boundary, outsideRadius;
        space.addAgent(diagonal, {21, 21});
        space.addAgent(outsideCircle, {24, 24});
        space.addAgent(boundary, {25, 15});
        space.addAgent(outsideRadius, {std::nextafter(25.f, 40.f), 15});
        auto identities = [](const auto& neighbors) {
            std::set<Agent*> result;
            for (auto entry : neighbors) result.insert(std::get<Agent*>(entry));
            return result;
        };
        EXPECT_EQ(identities(space.getNeighbors({15, 15}, 10, true, true)),
                  (std::set<Agent*>{&diagonal, &boundary}));
        EXPECT_EQ(identities(space.getNeighbors({15, 15}, 10, false, true)),
                  (std::set<Agent*>{&diagonal, &outsideCircle, &boundary}));
    }
}




}

namespace test::membership_continuous {
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


TEST(MembershipTest, ContinuousRemovalUsesIdentityAndMovesWithinBuckets) {
    agh::ContinuousSpace<RealAgent> field(2, 2, 1), other(2, 2, 1);
    RealAgent a, b, foreign;
    field.addAgent(a, {0.1f, 0.1f});
    field.addAgent(b, {0.1f, 0.1f});
    EXPECT_THROW(field.addAgent(a, {1, 0}), std::logic_error);
    EXPECT_THROW(other.addAgent(a, {1, 0}), std::logic_error);
    foreign.pos = agh::RealPoint{0.1f, 0.1f};
    field.removeAgent(foreign);
    EXPECT_TRUE(foreign.pos);
    EXPECT_THROW(field.moveAgent(foreign, {1, 0}), std::logic_error);
    EXPECT_THROW(field.moveAgent(a, {2, 0}), std::out_of_range);
    EXPECT_EQ(field.agentCount({0.1f, 0.1f}), 2);
    field.moveAgent(a, {0.2f, 0.2f});
    EXPECT_EQ(field.agentCount({0.1f, 0.1f}), 1);
    field.removeAgent(a);
    EXPECT_EQ(field.agentCount({0.1f, 0.1f}), 1);
    EXPECT_TRUE(b.pos);
    EXPECT_FALSE(a.pos);
    other.addAgent(a, {1, 1});
    field.moveAgent(b, {1, 0});
    EXPECT_EQ(field.agentCount({1, 0}), 1);
}

TEST(MembershipTest, MovingAndRemovalDoNotRequireValueEqualityContinuous) {    agh::ContinuousSpace<RealNoEquality> space(2, 2, 1);
    RealNoEquality b;
    space.moveAgent(b, {0, 0});
    space.moveAgent(b, {1, 1});
    space.removeAgent(b);
    EXPECT_FALSE(b.pos);

}
}

namespace test::spatial_safety_continuous {
namespace {
struct GridAgent { std::optional<agh::Point> pos; int value{}; };
struct RealAgent { std::optional<agh::RealPoint> pos; int value{}; };
}


TEST(SpaceSafetyTest, ContinuousRectangularCellsDoNotAlias) {
    for (auto size : {agh::Point{4, 2}, agh::Point{2, 4}}) {
        agh::ContinuousSpace<RealAgent> space(size.x, size.y, 1);
        std::array<RealAgent, 8> agents{};
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x)
                space.addAgent(agents[y * size.x + x], {x + 0.25f, y + 0.25f});
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x)
                EXPECT_EQ(space.agentCount({x + 0.25f, y + 0.25f}), 1);
    }
}

TEST(SpaceSafetyTest, RejectInvalidDimensionsAndUnrepresentableSizesContinuousSpace) {
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, 0), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, -1), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, std::numeric_limits<float>::infinity()), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(std::numeric_limits<float>::quiet_NaN(), 2, 1), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, std::numeric_limits<float>::min()), std::length_error);
}

TEST(SpaceSafetyTest, ValidateContinuousInputsAndNormalizeToroidalAccess) {
    agh::ContinuousSpace<RealAgent> bounded(4, 2, 0.7f);
    RealAgent agent;
    const float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_THROW(bounded.addAgent(agent, {nan, 0}), std::invalid_argument);
    EXPECT_THROW(bounded.addAgent(agent, {4, 0}), std::out_of_range);
    EXPECT_FALSE(agent.pos);
    EXPECT_THROW((void)bounded.getNeighbors({0, 0}, -1), std::invalid_argument);
    EXPECT_THROW((void)bounded.getNeighbors({0, 0}, nan), std::invalid_argument);
    EXPECT_THROW((void)bounded.getNeighbors({nan, 0}, 1), std::invalid_argument);
    agh::ContinuousSpace<RealAgent> torus(4, 2, 0.7f, true);
    torus.addAgent(agent, {-0.25f, -0.5f});
    EXPECT_EQ(*agent.pos, (agh::RealPoint{3.75f, 1.5f}));
    EXPECT_EQ(torus.agentCount({-0.25f, -0.5f}), 1);
    EXPECT_EQ(torus.toToroidal({4, 2}), (agh::RealPoint{0, 0}));
}
}

namespace test::api_continuousspace {
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
static_assert(acceptsVisitor<agh::ContinuousSpace<R, S>, EveryAgent>);
static_assert(!acceptsVisitor<agh::ContinuousSpace<R, S>, OnlyR>);
static_assert(acceptsAnyMembership<agh::ContinuousSpace<R>, R>);
static_assert(!acceptsAnyMembership<agh::ContinuousSpace<R>, S>);
TEST(ApiContractTest, ContinuousTraversalRejectStructuralMutations) {    agh::ContinuousSpace<R> space(2, 2, 1);
    R r;
    space.addAgent(r, {0, 0});
    EXPECT_THROW(space.apply([&](auto&) { space.moveAgent(r, {1, 1}); }), std::logic_error);
    EXPECT_THROW(space.apply([&](auto&) { space.removeAgent(r); }), std::logic_error);
    space.apply([](R& agent) { ++agent.value; });
    EXPECT_EQ(r.value, 1);
    space.removeAgent(r);
    EXPECT_FALSE(r.pos);

}
}
