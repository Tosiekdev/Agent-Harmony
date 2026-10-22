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

TEST(SpaceSafetyTest, ToroidalNeighborhoodsContainUniqueCellsAcrossConsumers) {
    for (auto size : {agh::Point{1, 1}, agh::Point{4, 2}, agh::Point{2, 4}}) {
        agh::Field<GridAgent> field(size.x, size.y, true);
        agh::MultiagentField<GridAgent> multi(size.x, size.y, true);
        agh::ValueLayer<int> values(size.x, size.y, true);
        std::array<GridAgent, 8> singles{}, multiples{};
        for (int y = 0; y < size.y; ++y) for (int x = 0; x < size.x; ++x) {
            const int i = y * size.x + x;
            field.addAgent(singles[i], {x, y});
            multi.addAgent(multiples[i], {x, y});
            values.setOnRead({x, y}, i);
        }
        for (int radius : {0, 1, 7}) for (bool moore : {false, true}) for (bool center : {false, true}) {
            std::vector<agh::Point> expected;
            std::vector<int> expectedValues;
            for (int y = 0; y < size.y; ++y) for (int x = 0; x < size.x; ++x) {
                const int dx = std::min(x, size.x - x), dy = std::min(y, size.y - y);
                if (!center && x == 0 && y == 0) continue;
                if ((moore ? std::max(dx, dy) : dx + dy) > radius) continue;
                expected.push_back({x, y});
                expectedValues.push_back(y * size.x + x);
            }
            EXPECT_EQ(field.getNeighborhood({size.x, -size.y}, radius, moore, center), expected);
            EXPECT_EQ(multi.getNeighborhood({0, 0}, radius, moore, center), expected);
            EXPECT_EQ(values.getNeighborhood({0, 0}, radius, moore, center), expected);
            EXPECT_EQ(field.getNeighbors({0, 0}, radius, moore, center).size(), expected.size());
            EXPECT_EQ(multi.getNeighbors({0, 0}, radius, moore, center).size(), expected.size());
            EXPECT_EQ(values.getNeighbors({0, 0}, radius, moore, center), expectedValues);
        }
    }
}

TEST(SpaceSafetyTest, HugeDiscreteRadiusIsBoundedByDomain) {
    agh::ValueLayer<int> bounded(4, 2), torus(4, 2, true);
    EXPECT_EQ(bounded.getNeighborhood({0, 0}, INT_MAX, false, true).size(), 8);
    EXPECT_EQ(torus.getNeighborhood({0, 0}, INT_MAX, true, false).size(), 7);
}

TEST(SpaceSafetyTest, SmallNeighborhoodInLargeVirtualGrid) {
    // A grid need not allocate its full domain to support local queries.
    struct VirtualGrid {
        int getWidth() const { return INT_MAX; }
        int getHeight() const { return INT_MAX - 2; }
        bool isToroidal() const { return true; }
        bool outOfBounds(agh::Point p) const {
            return p.x < 0 || p.x >= getWidth() || p.y < 0 || p.y >= getHeight();
        }
        agh::Point toToroidal(agh::Point p) const {
            return agh::convertToToroidal(p, getWidth(), getHeight());
        }
    } grid;
    const std::vector<agh::Point> moore{
        {1, 0}, {INT_MAX - 1, 0},
        {0, 1}, {1, 1}, {INT_MAX - 1, 1},
        {0, INT_MAX - 3}, {1, INT_MAX - 3}, {INT_MAX - 1, INT_MAX - 3}};
    const std::vector<agh::Point> vonNeumann{
        {1, 0}, {INT_MAX - 1, 0}, {0, 1}, {0, INT_MAX - 3}};
    for (bool useMoore : {false, true}) {
        const auto& expected = useMoore ? moore : vonNeumann;
        EXPECT_EQ(agh::visitNeighborhood(grid, {0, 0}, 1, useMoore, false,
            [](agh::Point p) { return p; }), expected);
        EXPECT_EQ((agh::visitNeighbors<VirtualGrid, agh::Point>(grid, {0, 0}, 1, useMoore, false,
            [](agh::Point p, std::vector<agh::Point>& result) { result.push_back(p); })), expected);
    }
}

TEST(SpaceSafetyTest, DiscreteNeighborhoodsMatchFullScanReference) {
    for (int width = 1; width <= 7; ++width) for (int height = 1; height <= 6; ++height)
    for (bool toroidal : {false, true}) {
        agh::ValueLayer<int> layer(width, height, toroidal);
        agh::Field<GridAgent> field(width, height, toroidal);
        agh::MultiagentField<GridAgent> multi(width, height, toroidal);
        std::vector<GridAgent> singles(width * height), multiples(width * height);
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            layer.setOnRead({x, y}, y * width + x);
            field.addAgent(singles[y * width + x], {x, y});
            multi.addAgent(multiples[y * width + x], {x, y});
        }
        for (int py = 0; py < height; ++py) for (int px = 0; px < width; ++px)
        for (int radius : {0, 1, 2, 3, 6, INT_MAX})
        for (bool moore : {false, true}) for (bool center : {false, true}) {
            std::vector<agh::Point> expected;
            std::vector<int> expectedValues;
            std::vector<std::variant<GridAgent*>> expectedSingles, expectedMultiples;
            for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
                int dx = std::abs(x - px), dy = std::abs(y - py);
                if (toroidal) {
                    dx = std::min(dx, width - dx);
                    dy = std::min(dy, height - dy);
                }
                if (!center && x == px && y == py) continue;
                if ((moore ? std::max(dx, dy) : dx + dy) > radius) continue;
                expected.push_back({x, y});
                expectedValues.push_back(y * width + x);
                expectedSingles.push_back(&singles[y * width + x]);
                expectedMultiples.push_back(&multiples[y * width + x]);
            }
            const agh::Point query = toroidal ? agh::Point{px - width, py + height} : agh::Point{px, py};
            ASSERT_EQ(layer.getNeighborhood(query, radius, moore, center), expected)
                << width << 'x' << height << " center=" << px << ',' << py << " radius=" << radius
                << " toroidal=" << toroidal << " moore=" << moore << " includeCenter=" << center;
            ASSERT_EQ(layer.getNeighbors(query, radius, moore, center), expectedValues);
            ASSERT_EQ(field.getNeighbors(query, radius, moore, center), expectedSingles);
            ASSERT_EQ(multi.getNeighbors(query, radius, moore, center), expectedMultiples);
        }
    }
}
