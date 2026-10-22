#include <gtest/gtest.h>
#include "space/Field.hpp"
#include "space/MultiagentField.hpp"
#include "space/ContinuousSpace.hpp"

namespace {
struct Agent {
    std::optional<agh::Point> pos;
    bool operator==(const Agent&) const { return true; }
};
struct RealAgent {
    std::optional<agh::RealPoint> pos;
    bool operator==(const RealAgent&) const { return true; }
};
struct NoEquality { std::optional<agh::Point> pos; };
struct RealNoEquality { std::optional<agh::RealPoint> pos; };
}

TEST(MembershipTest, FieldRejectsDuplicatesAndForeignOperations) {
    agh::Field<Agent> field(2, 2), other(2, 2);
    Agent a, b, foreign;
    EXPECT_TRUE(field.addAgent(a, {0, 0}));
    EXPECT_TRUE(field.addAgent(b, {1, 0}));
    EXPECT_FALSE(field.addAgent(a, {0, 1}));
    EXPECT_FALSE(other.addAgent(a, {0, 1}));
    foreign.pos = agh::Point{0, 0};
    field.removeAgent(foreign);
    EXPECT_TRUE(foreign.pos);
    ASSERT_TRUE(field.getAgent({0, 0}));
    EXPECT_EQ(std::get<Agent*>(*field.getAgent({0, 0})), &a);
    EXPECT_THROW(field.moveAgent(foreign, {1, 1}), std::logic_error);
    EXPECT_TRUE(field.moveAgent(a, {0, 0}));
    EXPECT_FALSE(field.moveAgent(a, {1, 0}));
    EXPECT_EQ(*a.pos, (agh::Point{0, 0}));
    EXPECT_THROW(field.moveAgent(a, {2, 0}), std::out_of_range);
    EXPECT_TRUE(field.moveAgent(a, {1, 1}));
    field.removeAgent(a);
    field.removeAgent(a);
    EXPECT_FALSE(a.pos);
    EXPECT_TRUE(other.addAgent(a, {0, 0}));
}

TEST(MembershipTest, MultiagentRemovalUsesIdentity) {
    agh::MultiagentField<Agent> field(2, 2), other(2, 2);
    Agent a, b, foreign;
    field.addAgent(a, {0, 0});
    field.addAgent(b, {0, 0});
    EXPECT_THROW(field.addAgent(a, {1, 0}), std::logic_error);
    EXPECT_THROW(other.addAgent(a, {1, 0}), std::logic_error);
    foreign.pos = agh::Point{0, 0};
    field.removeAgent(foreign);
    EXPECT_TRUE(foreign.pos);
    EXPECT_THROW(field.moveAgent(foreign, {1, 0}), std::logic_error);
    EXPECT_THROW(field.moveAgent(a, {2, 0}), std::out_of_range);
    EXPECT_EQ(field.agentCount({0, 0}), 2);
    field.moveAgent(a, {0, 0});
    field.removeAgent(a);
    ASSERT_EQ(field.getAgents({0, 0}).size(), 1);
    EXPECT_EQ(std::get<Agent*>(field.getAgents({0, 0})[0]), &b);
    EXPECT_TRUE(b.pos);
    EXPECT_FALSE(a.pos);
    other.addAgent(a, {1, 1});
    field.moveAgent(b, {1, 0});
    EXPECT_TRUE(field.isEmpty({0, 0}));
}

TEST(MembershipTest, ContinuousRemovalUsesIdentityAndMovesWithinBuckets) {
    agh::ContinuousSpace<RealAgent> field(2, 2, 1), other(2, 2, 1);
    RealAgent a, b, foreign;
    field.addAgent(a, {0.1f, 0.1f});
    field.addAgent(b, {0.1f, 0.1f});
    EXPECT_THROW(field.addAgent(a, {1, 0}), std::logic_error);
    EXPECT_THROW(other.addAgent(a, {1, 0}), std::logic_error);
    foreign.pos = agh::RealPoint{0.1f, 0.1f};
    field.removeAgent(foreign);
    EXPECT_TRUE(foreign.pos);
    EXPECT_THROW(field.moveAgent(foreign, {1, 0}), std::logic_error);
    EXPECT_THROW(field.moveAgent(a, {2, 0}), std::out_of_range);
    EXPECT_EQ(field.agentCount({0.1f, 0.1f}), 2);
    field.moveAgent(a, {0.2f, 0.2f});
    EXPECT_EQ(field.agentCount({0.1f, 0.1f}), 1);
    field.removeAgent(a);
    EXPECT_EQ(field.agentCount({0.1f, 0.1f}), 1);
    EXPECT_TRUE(b.pos);
    EXPECT_FALSE(a.pos);
    other.addAgent(a, {1, 1});
    field.moveAgent(b, {1, 0});
    EXPECT_EQ(field.agentCount({1, 0}), 1);
}

TEST(MembershipTest, MovingAndRemovalDoNotRequireValueEquality) {
    agh::MultiagentField<NoEquality> grid(2, 2);
    NoEquality a;
    grid.moveAgent(a, {0, 0});
    grid.moveAgent(a, {1, 1});
    grid.removeAgent(a);
    EXPECT_FALSE(a.pos);
    agh::ContinuousSpace<RealNoEquality> space(2, 2, 1);
    RealNoEquality b;
    space.moveAgent(b, {0, 0});
    space.moveAgent(b, {1, 1});
    space.removeAgent(b);
    EXPECT_FALSE(b.pos);
}
