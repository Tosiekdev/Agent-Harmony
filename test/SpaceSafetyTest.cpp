#include <gtest/gtest.h>
#include <array>
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
