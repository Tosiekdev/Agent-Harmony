#include <gtest/gtest.h>
#include <array>
#include <climits>
#include <limits>
#include <utility>
#include "space/Field.hpp"
#include "space/MultiagentField.hpp"
#include "space/ContinuousSpace.hpp"
#include "space/ValueLayer.hpp"

namespace {
struct GridAgent { std::optional<agh::Point> pos; int value{}; };
struct RealAgent { std::optional<agh::RealPoint> pos; int value{}; };
}

TEST(SpaceSafetyTest, RectangularEmptySets) {
    for (auto size : {agh::Point{1, 1}, agh::Point{4, 2}, agh::Point{2, 4}}) {
        agh::Field<GridAgent> single(size.x, size.y);
        agh::MultiagentField<GridAgent> multi(size.x, size.y);
        std::vector<agh::Point> expected;
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x) expected.push_back({x, y});
        EXPECT_EQ(single.getEmpty(), expected);
        EXPECT_EQ(multi.getEmpty(), expected);
        GridAgent a, b;
        single.addAgent(a, {0, 0});
        multi.addAgent(b, {0, 0});
        expected.erase(expected.begin());
        EXPECT_EQ(single.getEmpty(), expected);
        EXPECT_EQ(multi.getEmpty(), expected);
    }
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

TEST(SpaceSafetyTest, RejectInvalidDimensionsAndUnrepresentableSizes) {
    EXPECT_THROW(agh::Field<GridAgent>(0, 2), std::invalid_argument);
    EXPECT_THROW(agh::MultiagentField<GridAgent>(2, -1), std::invalid_argument);
    EXPECT_THROW(agh::ValueLayer<int>(-1, 2), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, 0), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, -1), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, std::numeric_limits<float>::infinity()), std::invalid_argument);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(std::numeric_limits<float>::quiet_NaN(), 2, 1), std::invalid_argument);
    EXPECT_THROW(agh::Field<GridAgent>(INT_MAX, INT_MAX), std::length_error);
    EXPECT_THROW(agh::ContinuousSpace<RealAgent>(2, 2, std::numeric_limits<float>::min()), std::length_error);
}

TEST(SpaceSafetyTest, RejectInvalidCoordinatesWithoutChangingMembership) {
    agh::Field<GridAgent> field(2, 4);
    agh::MultiagentField<GridAgent> multi(2, 4);
    agh::ValueLayer<int> values(2, 4);
    GridAgent a, b;
    EXPECT_THROW(field.addAgent(a, {2, 0}), std::out_of_range);
    EXPECT_THROW(multi.addAgent(b, {-1, 0}), std::out_of_range);
    EXPECT_FALSE(a.pos);
    EXPECT_FALSE(b.pos);
    EXPECT_THROW(std::as_const(field).getAgent({0, 4}), std::out_of_range);
    EXPECT_THROW(std::as_const(multi).getAgents({2, 0}), std::out_of_range);
    EXPECT_THROW(values.set({2, 0}, 1), std::out_of_range);
    EXPECT_THROW(values.getFromWrite({0, -1}), std::out_of_range);
    EXPECT_THROW(values.setOnRead({0, 4}, 1), std::out_of_range);
    EXPECT_THROW(values.get({-1, 0}), std::out_of_range);
    EXPECT_THROW((void)field.getNeighborhood({0, 0}, -1), std::invalid_argument);
    EXPECT_THROW((void)multi.getNeighbors({0, 0}, -1, true, false), std::invalid_argument);
    EXPECT_THROW((void)values.getNeighbors({0, 0}, -1, false, false), std::invalid_argument);
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

TEST(SpaceSafetyTest, NormalizeDiscreteToroidalAccess) {
    agh::Field<GridAgent> field(4, 2, true);
    agh::MultiagentField<GridAgent> multi(4, 2, true);
    agh::ValueLayer<int> values(4, 2, true);
    GridAgent a, b;
    field.addAgent(a, {-1, -1});
    multi.addAgent(b, {4, 2});
    EXPECT_EQ(*a.pos, (agh::Point{3, 1}));
    EXPECT_EQ(*b.pos, (agh::Point{0, 0}));
    EXPECT_FALSE(field.isEmpty({-1, -1}));
    EXPECT_EQ(multi.getAgents({4, 2}).size(), 1);
    values.set({-1, -1}, 7);
    values.swap();
    EXPECT_EQ(values.get({3, 1}), 7);
    EXPECT_EQ(field.toToroidal({INT_MIN, INT_MAX}), (agh::Point{0, 1}));
}
