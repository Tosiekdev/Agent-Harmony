#include <gtest/gtest.h>

#include "../space/ValueLayer.hpp"

namespace test::value_layer {
TEST(ValueLayerTest, GetSetRead) {
    abmf::IntValueLayer layer(2, 2);
    layer.setOnRead({0, 0}, 2);
    EXPECT_EQ(layer.get({0, 0}), 2);
}

TEST(ValueLayerTest, GetSetWrite) {
    abmf::IntValueLayer layer(2, 2);
    layer.set({0, 0}, 2);
    EXPECT_EQ(layer.getFromWrite({0, 0}), 2);
}

TEST(ValueLayerTest, ApplyUnary) {
    abmf::IntValueLayer layer(2, 2, false);
    layer.apply([](int& value) { value += 2; });
    layer.swap();
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), 2);
        }
    }
}

TEST(ValueLayerTest, Transform) {
    abmf::RealValueLayer layer(2, 2, false, 1.);
    layer.transform([](abmf::Point p, double& value) { value += p.x + p.y; });
    layer.swap();
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), i+j+1.);
        }
    }
}

TEST(ValueLayerTest, Apply) {
    abmf::RealValueLayer layer(2, 2, false, 1.);
    std::vector<double> values;
    layer.apply([&values](double value) mutable { values.push_back(value); });
    EXPECT_EQ(values.size(), 2*2);
    for (auto v : values) {
        EXPECT_EQ(1, v);
    }
}

TEST(ValueLayerTest, ForEach) {
    abmf::ValueLayer<size_t> layer(2, 2, false, 1.);
    std::vector<size_t> values;
    layer.forEach([&values](abmf::Point p, size_t value) mutable { values.push_back(value + p.x + p.y); });
    EXPECT_EQ(values.size(), 2*2);
    for(size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            EXPECT_EQ(values[i*2 + j], 1 + i + j);
        }
    }
}

TEST(ValueLayerTest, GetNeighborhood) {
    abmf::RealValueLayer layer(3, 3, false, 1.);
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
