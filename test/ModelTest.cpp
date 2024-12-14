#include <gtest/gtest.h>

#include <optional>

#include "../include/model/Model.hpp"

namespace test::model {
struct MyAgent {
    explicit MyAgent(const int pId) : id(pId) {}
    int id;
    bool isActive() {
        return true;
    }
};

struct NotMyAgent {
    explicit NotMyAgent(const int pId) : id(pId) {}
    int id;
    bool isActive() {
        return true;
    }
};

struct MyModel : abmf::Model<MyAgent> {};

struct MyModel2 : abmf::Model<MyAgent, NotMyAgent> {};

TEST(ModelTest, AddingAgent) {
    MyModel2 m;
    MyAgent a1{1};
    NotMyAgent a2{2};
    m.addAgent(a1);
    m.addAgent(a2);
    ASSERT_EQ(m.agentCount(), 2);
    ASSERT_EQ(m.agentCount<MyAgent>(), 1);
    ASSERT_EQ(m.agentCount<NotMyAgent>(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
    EXPECT_EQ(m.getAgents<NotMyAgent>()[0].id, 2);
}

TEST(ModelTest, EmplacingAgent) {
    MyModel m;
    m.emplaceAgent<MyAgent>(1);
    ASSERT_EQ(m.agentCount(), 1);
    ASSERT_EQ(m.agentCount<MyAgent>(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
}

TEST(ModelTest, AddingAgentDifferentTypes) {
    MyModel2 m;
    m.emplaceAgent<MyAgent>(1);
    m.emplaceAgent<NotMyAgent>(2);
    ASSERT_EQ(m.agentCount(), 2);
    ASSERT_EQ(m.agentCount<MyAgent>(), 1);
    ASSERT_EQ(m.agentCount<NotMyAgent>(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
    EXPECT_EQ(m.getAgents<NotMyAgent>()[0].id, 2);
}
}
