#include "AgentHarmony.hpp"
#include <array>
#include <climits>
#include <limits>
#include <utility>
#include <gtest/gtest.h>

#include "../include/space/ValueLayer.hpp"

namespace test::value_layer {
TEST(ValueLayerTest, GetSetRead) {
    agh::IntValueLayer layer(2, 2);
    layer.setOnRead({0, 0}, 2);
    EXPECT_EQ(layer.get({0, 0}), 2);
}

TEST(ValueLayerTest, GetSetWrite) {
    agh::IntValueLayer layer(2, 2);
    layer.set({0, 0}, 2);
    EXPECT_EQ(layer.getFromWrite({0, 0}), 2);
}

TEST(ValueLayerTest, ApplyUnary) {
    agh::IntValueLayer layer(2, 2, false);
    layer.apply([](int& value) { value += 2; });
    layer.swap();
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), 2);
        }
    }
}

TEST(ValueLayerTest, Transform) {
    agh::RealValueLayer layer(2, 2, false, 1.);
    layer.transform([](agh::Point p, double& value) { value += p.x + p.y; });
    layer.swap();
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), i+j+1.);
        }
    }
}

TEST(ValueLayerTest, Apply) {
    agh::RealValueLayer layer(2, 2, false, 1.);
    std::vector<double> values;
    layer.apply([&values](double value) mutable { values.push_back(value); });
    EXPECT_EQ(values.size(), 2*2);
    for (auto v : values) {
        EXPECT_EQ(1, v);
    }
}

TEST(ValueLayerTest, TransformWithFunctionTakingCopy) {
    agh::ValueLayer<size_t> layer(2, 2, false, 1.);
    std::vector<size_t> values;
    layer.transform([&values](agh::Point p, size_t value) mutable { values.push_back(value + p.x + p.y); });
    EXPECT_EQ(values.size(), 2*2);
    for(size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            EXPECT_EQ(values[i*2 + j], 1 + i + j);
        }
    }
}

TEST(ValueLayerTest, GetNeighborhood) {
    agh::RealValueLayer layer(3, 3, false, 1.);
    auto moore = layer.getNeighborhood({1, 1}, 1, true, false);
    auto mooreCenter = layer.getNeighborhood({1, 1}, 1, true, true);

    auto vonNeumann = layer.getNeighborhood({1, 1}, 1, false, false);
    auto vonNeumannCenter = layer.getNeighborhood({1, 1}, 1, false, true);

    EXPECT_EQ(moore.size(), 8);
    EXPECT_EQ(mooreCenter.size(), 9);
    EXPECT_EQ(vonNeumann.size(), 4);
    EXPECT_EQ(vonNeumannCenter.size(), 5);
}
}

namespace test::spatial_safety_values {
namespace {
struct GridAgent { std::optional<agh::Point> pos; int value{}; };
struct RealAgent { std::optional<agh::RealPoint> pos; int value{}; };
}


TEST(SpaceSafetyTest, RejectInvalidDimensionsAndUnrepresentableSizesValueLayer) {
    EXPECT_THROW(agh::ValueLayer<int>(-1, 2), std::invalid_argument);
}

TEST(SpaceSafetyTest, RejectInvalidCoordinatesWithoutChangingMembershipValueLayer) {
    agh::ValueLayer<int> values(2, 4);
    EXPECT_THROW(values.set({2, 0}, 1), std::out_of_range);
    EXPECT_THROW(values.getFromWrite({0, -1}), std::out_of_range);
    EXPECT_THROW(values.setOnRead({0, 4}, 1), std::out_of_range);
    EXPECT_THROW(values.get({-1, 0}), std::out_of_range);
    EXPECT_THROW((void)values.getNeighbors({0, 0}, -1, false, false), std::invalid_argument);
}

TEST(SpaceSafetyTest, NormalizeDiscreteToroidalAccessValueLayer) {
    agh::ValueLayer<int> values(4, 2, true);
    values.set({-1, -1}, 7);
    values.swap();
    EXPECT_EQ(values.get({3, 1}), 7);
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
}
