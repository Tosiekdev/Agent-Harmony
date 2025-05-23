#include <gtest/gtest.h>

#include "../include/space/Field.hpp"

namespace test::field {
struct MyAgent {
    std::optional<agh::Point> pos;
    int value{};
};

TEST(FieldTest, MoveAgent) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.moveAgent(agent, {1, 0});
    EXPECT_EQ(*agent.pos, agh::Point(1,0));
    EXPECT_TRUE(field.isEmpty(agh::Point(1, 1)));
}

TEST(FieldTest, Apply) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.apply([](auto& a) { a.value += 2; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, Transform) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.transform([](agh::Point p, auto& a) { a.value += p.x + p.y; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, RemoveAgent) {
    using FieldT = agh::Field<MyAgent>;
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

TEST(FieldTest, GetEmpty) {
    using FieldT = agh::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    EXPECT_EQ(field.getEmpty().size(), 4);
    field.addAgent(agent, {1, 1});
    EXPECT_EQ(field.getEmpty().size(), 3);
    field.addAgent(agent, {1, 0});
    EXPECT_EQ(field.getEmpty().size(), 2);
    field.addAgent(agent, {0, 1});
    EXPECT_EQ(field.getEmpty().size(), 1);
    field.addAgent(agent, {0, 0});
    EXPECT_EQ(field.getEmpty().size(), 0);
}

TEST(FieldTest, ToToroidal) {
    using FieldT = agh::Field<MyAgent>;
    const FieldT field(2,2);

    agh::Point p = field.toToroidal({-2, -2});
    EXPECT_EQ(p.x, 0);
    EXPECT_EQ(p.y, 0);

    p = field.toToroidal({-3, -3});
    EXPECT_EQ(p.x, 1);
    EXPECT_EQ(p.y, 1);

    p = field.toToroidal({-4, -4});
    EXPECT_EQ(p.x, 0);
    EXPECT_EQ(p.y, 0);

    p = field.toToroidal({-5, -4});
    EXPECT_EQ(p.x, 1);
    EXPECT_EQ(p.y, 0);
}
}
