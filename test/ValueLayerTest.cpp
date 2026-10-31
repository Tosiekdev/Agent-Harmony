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
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), 2);
        }
    }
}

TEST(ValueLayerTest, Transform) {
    agh::RealValueLayer layer(2, 2, false, 1.);
    layer.transform([](agh::Point p, double& value) { value += p.x + p.y; });
    layer.swap();
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
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
    EXPECT_THROW(agh::ValueLayer<int>(0, 2), std::invalid_argument);
}

TEST(SpaceSafetyTest, RejectInvalidCoordinatesWithoutChangingMembershipValueLayer) {
    agh::ValueLayer<int> values(2, 4);
    EXPECT_THROW(values.set({2, 0}, 1), std::out_of_range);
    EXPECT_THROW(values.getFromWrite({0, 4}), std::out_of_range);
    EXPECT_THROW(values.setOnRead({0, 4}, 1), std::out_of_range);
    EXPECT_THROW(values.get({2, 0}), std::out_of_range);
}

TEST(SpaceSafetyTest, NormalizeDiscreteToroidalAccessValueLayer) {
    agh::ValueLayer<int> values(4, 2, true);
    values.set({7, 3}, 7);
    values.swap();
    EXPECT_EQ(values.get({3, 1}), 7);
}




}

static_assert(std::is_same_v<decltype(agh::Point{}.x), std::size_t>);
static_assert(std::is_same_v<decltype(std::declval<agh::ValueLayer<int>>().getWidth()), std::size_t>);

TEST(ValueLayerTest, UnsignedLimitsDoNotWrapStorageOrOffsets) {
    const auto maximum = std::numeric_limits<std::size_t>::max();
    EXPECT_THROW(agh::ValueLayer<int>(maximum, 2), std::length_error);
    EXPECT_THROW(agh::convertToToroidal({0, 0}, 0, 2), std::invalid_argument);
    agh::ValueLayer<int> values(4, 2);
    EXPECT_TRUE(values.getNeighborhood({maximum, maximum}, 1, true, false).empty());
    EXPECT_THROW((void)values.getNeighborhood({0, 0}, maximum, true, false), std::length_error);
    EXPECT_EQ(values.getNeighborhood({4, 0}, 1, false, false), (std::vector<agh::Point>{{3, 0}}));
}
