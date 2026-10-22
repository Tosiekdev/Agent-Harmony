#include <gtest/gtest.h>
#include <array>
#include <random>
#include <set>
#include "space/ContinuousSpace.hpp"

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
