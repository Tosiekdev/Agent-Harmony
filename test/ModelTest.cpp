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

struct MyModel : agh::Model<MyAgent> {};

struct MyModel2 : agh::Model<MyAgent, NotMyAgent> {};

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

namespace {
struct CountedAgent {
    static inline int copies = 0;
    CountedAgent() = default;
    CountedAgent(const CountedAgent&) { ++copies; }
    bool isActive() const { return true; }
};
struct NoncopyableAgent {
    NoncopyableAgent() = default;
    NoncopyableAgent(const NoncopyableAgent&) = delete;
    bool isActive() const { return true; }
};
}

TEST(ModelCountTest, CountingDoesNotCopyAgents) {
    agh::Model<CountedAgent> model;
    EXPECT_EQ(std::as_const(model).agentCount(), 0);
    for (int i = 0; i < 3; ++i) model.emplaceAgent<CountedAgent>();
    CountedAgent::copies = 0;
    EXPECT_EQ(std::as_const(model).agentCount(), 3);
    EXPECT_EQ(CountedAgent::copies, 0);
}

TEST(ModelCountTest, NoncopyableMixedAgentsCanBeCounted) {
    agh::Model<CountedAgent, NoncopyableAgent> model;
    model.emplaceAgent<NoncopyableAgent>();
    model.emplaceAgent<CountedAgent>();
    EXPECT_EQ(std::as_const(model).agentCount(), 2);
}
