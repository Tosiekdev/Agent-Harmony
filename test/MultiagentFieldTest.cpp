#include <gtest/gtest.h>

#include <array>

#include "../include/space/MultiagentField.hpp"

namespace test::multiagent_field {
struct MyAgent {
    int id{};
    std::optional<abmf::Point> pos;
    int value{};

    bool operator==(const MyAgent& rhs) const {
        return id == rhs.id;
    }
};

TEST(MultiagentFieldTest, AddAgent) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    EXPECT_EQ(field.getAgents({0,0}).size(), 2);
}

TEST(MultiagentFieldTest, Apply) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.apply([](auto& a) { a.value += 2; });
    EXPECT_EQ(agent.value, 2);
    EXPECT_EQ(agent2.value, 2);
}

TEST(MultiagentFieldTest, Transform) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    MyAgent agent3{3};
    Field field(2, 2);
    field.addAgent(agent, {0, 1});
    field.addAgent(agent2, {1, 1});
    field.addAgent(agent3, {1, 1});
    field.transform([](abmf::Point p, auto& a) {
        a.value += p.x + p.y;
    });
    EXPECT_EQ(agent.value, 1);
    EXPECT_EQ(agent2.value, 2);
    EXPECT_EQ(agent3.value, 2);
}

TEST(MultiagentFieldTest, RemoveAgent) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.removeAgent(agent);
    EXPECT_EQ(field.getAgents({0,0}).size(), 1);
    EXPECT_FALSE(agent.pos.has_value());
}

TEST(MultiagentFieldTest, RemoveAgents) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent{1};
    MyAgent agent2{2};
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.removeAgents({0, 0});
    EXPECT_EQ(field.getAgents({0,0}).size(), 0);
    EXPECT_FALSE(agent.pos.has_value());
    EXPECT_FALSE(agent2.pos.has_value());
}

TEST(MultiagentFieldTest, GetNeighborhood) {
    using Field = abmf::MultiagentField<MyAgent>;
    Field field(3, 3);
    auto moore = field.getNeighborhood({1, 1}, 1, true, false);
    auto mooreCenter = field.getNeighborhood({1, 1}, 1, true, true);

    auto vonNeumann = field.getNeighborhood({1, 1}, 1, false, false);
    auto vonNeumannCenter = field.getNeighborhood({1, 1}, 1, false, true);

    EXPECT_EQ(moore.size(), 8);
    EXPECT_EQ(mooreCenter.size(), 9);
    EXPECT_EQ(vonNeumann.size(), 4);
    EXPECT_EQ(vonNeumannCenter.size(), 5);
}

namespace ngh {
using Field = abmf::MultiagentField<MyAgent>;
Field field(3, 3);
std::array<MyAgent, 9> agents;

void assignAgents() {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            field.addAgent(agents[y * 3 + x], {x, y});
            agents[y * 3 + x].id = y * 3 + x;
        }
    }
}

TEST(MultiagentFieldTest, GetMooreNeighbors) {
    assignAgents();
    auto moore = field.getNeighbors({1, 1}, 1, true, false);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(moore.size(), 8);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            const auto index = y * 3 + x;
            if (index < 4) {
                EXPECT_TRUE(equalityCheck(moore[index], agents[index]));
            }
            else if (index > 4) {
                EXPECT_TRUE(equalityCheck(moore[index-1], agents[index]));
            }
        }
    }
}

TEST(MultiagentFieldTest, GetMooreNeighborsNoCenter) {
    assignAgents();
    auto mooreCenter = field.getNeighbors({1, 1}, 1, true, true);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(mooreCenter.size(), 9);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            const auto index = y * 3 + x;
            EXPECT_TRUE(equalityCheck(mooreCenter[index], agents[index]));
        }
    }
}

TEST(MultiagentFieldTest, GetVonNeumannNeighbors) {
    assignAgents();
    auto vonNeumann = field.getNeighbors({1, 1}, 1, false, false);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(vonNeumann.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(equalityCheck(vonNeumann[i], agents[2*i+1]));
    }
}

TEST(MultiagentFieldTest, GetVonNeumannNeighborsNoCenter) {
    assignAgents();
    auto vonNeumannCenter = field.getNeighbors({1, 1}, 1, false, true);

    auto equalityCheck = [](Field::AgentT agent, MyAgent& expect) {
        return std::visit([&](auto a) { return *a == expect; }, agent);
    };

    ASSERT_EQ(vonNeumannCenter.size(), 5);
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[0], agents[1]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[1], agents[3]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[2], agents[4]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[3], agents[5]));
    EXPECT_TRUE(equalityCheck(vonNeumannCenter[4], agents[7]));
}
}
}
