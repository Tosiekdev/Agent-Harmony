#include <gtest/gtest.h>

#include "../space/Field.hpp"

namespace test::field {
struct MyAgent {
    std::optional<abmf::Point> pos;
    int value{};
};

TEST(FieldTest, MoveAgent) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.moveAgent(agent, {1, 0});
    EXPECT_EQ(*agent.pos, abmf::Point(1,0));
    EXPECT_TRUE(field.isEmpty(abmf::Point(1, 1)));
}

TEST(FieldTest, Apply) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.apply([](auto& a) { a.value += 2; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, Transform) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.transform([](abmf::Point p, auto& a) { a.value += p.x + p.y; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, RemoveAgent) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    EXPECT_TRUE(field.getAgent({1,1}).has_value());
    field.removeAgent({1, 1});
    EXPECT_FALSE(field.getAgent({1,1}).has_value());
    field.addAgent(agent, {1, 1});
    EXPECT_TRUE(field.getAgent({1,1}).has_value());
    field.removeAgent(agent);
    EXPECT_FALSE(field.getAgent({1,1}).has_value());
}
}
