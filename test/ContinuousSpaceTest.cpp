#include <gtest/gtest.h>

#include "../include/space/ContinuousSpace.hpp"

namespace test::continuous {
struct Agent {
    std::optional<abmf::RealPoint> pos;
};

TEST(ContinuousSpaceTest, GetNeighbours) {
    abmf::ContinuousSpace<Agent> space(10.f, 10.f, 0.1f);
    Agent a{abmf::RealPoint{5.f, 5.f}};
    Agent b{abmf::RealPoint{3.f, 3.f}};

    auto nghs = space.getNeighbors(*a.pos, 3.f);
}
}
