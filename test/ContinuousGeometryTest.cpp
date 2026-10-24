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

TEST(ContinuousGeometryTest, PreserveBucketAndAgentOrderAcrossWrappedIntervals) {
    constexpr agh::RealPoint extent{4.3f, 2.2f};
    const std::array<agh::RealPoint, 12> locations{{
        {4.25f, 2.15f}, {0, 0}, {1.5f, 0.1f}, {1.49f, 0.1f},
        {3, 1.5f}, {2.99f, 1.49f}, {0, 0}, {0.05f, 2.15f},
        {4.25f, 0.05f}, {1.6f, 1.6f}, {2.1f, 1}, {0.8f, 0.9f}}};
    for (float discretization : {0.7f, 1.5f, 10.f}) for (bool torus : {false, true}) {
        agh::ContinuousSpace<Agent> space(extent.x, extent.y, discretization, torus);
        std::array<Agent, locations.size()> agents;
        for (std::size_t i = 0; i < agents.size(); ++i) space.addAgent(agents[i], locations[i]);
        for (auto origin : {agh::RealPoint{0, 0}, agh::RealPoint{0.1f, 0.1f},
                            agh::RealPoint{4.25f, 2.15f}, agh::RealPoint{2.15f, 1.1f}})
        for (float radius : {0.f, 0.2f, 0.7f, 1.09f, 1.1f, 2.14f, 2.15f, 3.f,
                             std::numeric_limits<float>::max()})
        for (bool euclidean : {false, true}) for (bool center : {false, true}) {
            std::vector<Agent*> expected, actual;
            for (auto& agent : agents) {
                if (!center && *agent.pos == origin) continue;
                double dx = std::abs(double(agent.pos->x) - origin.x);
                double dy = std::abs(double(agent.pos->y) - origin.y);
                if (torus) {
                    dx = std::min(dx, double(extent.x) - dx);
                    dy = std::min(dy, double(extent.y) - dy);
                }
                if ((euclidean ? std::hypot(dx, dy) : std::max(dx, dy)) <= radius)
                    expected.push_back(&agent);
            }
            // Characterize the existing row-major bucket traversal and insertion order.
            auto cell = [&](const Agent* agent) {
                return std::pair{std::floor(double(agent->pos->y) / discretization),
                                 std::floor(double(agent->pos->x) / discretization)};
            };
            std::stable_sort(expected.begin(), expected.end(),
                             [&](const Agent* a, const Agent* b) { return cell(a) < cell(b); });
            for (auto entry : space.getNeighbors(origin, radius, euclidean, center))
                actual.push_back(std::get<Agent*>(entry));
            ASSERT_EQ(actual, expected) << "torus=" << torus << " dx=" << discretization
                                       << " radius=" << radius << " euclidean=" << euclidean;
        }
    }
}
