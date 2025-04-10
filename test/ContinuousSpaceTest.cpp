#include <gtest/gtest.h>

#include "../include/space/ContinuousSpace.hpp"

namespace test::continuous {
struct Agent {
    std::optional<abmf::RealPoint> pos;
};

TEST(ContinuousSpaceTest, AddAgent) {
    abmf::ContinuousSpace<Agent> space(10.f, 10.f, 1.f);
    Agent a, b, c;

    space.addAgent(a, {5.f, 5.f});
    space.addAgent(b, {3.f, 3.f});
    space.addAgent(c, {2.f, 2.f});

    EXPECT_EQ(space.agentCount({5.f, 5.f}), 1);
    EXPECT_EQ(space.agentCount({3.f, 3.f}), 1);
    EXPECT_EQ(space.agentCount({2.f, 2.f}), 1);
}

TEST(ContinuousSpaceTest, GetNeighbours) {
    abmf::ContinuousSpace<Agent> space(10.f, 10.f, 1.f);
    Agent a, b, c;

    space.addAgent(a, {5.f, 5.f});
    space.addAgent(b, {3.f, 3.f});
    space.addAgent(c, {2.f, 2.f});

    auto nghs = space.getNeighbors(*a.pos, 3.f, true, true);
    EXPECT_EQ(nghs.size(), 1);

    nghs = space.getNeighbors(*a.pos, 3.f, false, true);
    EXPECT_EQ(nghs.size(), 2);
}
}
