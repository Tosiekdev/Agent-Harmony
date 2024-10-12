#include <gtest/gtest.h>

#include "../Model.hpp"

struct MyAgent {
    int id;
    bool active{true};

    bool isActive() const {
        return active;
    }

    template<typename T>
    void step(T& model) {
        std::cout << id;
    }
};

// Demonstrate some basic assertions.
TEST(AgentsTest, AddingAgent) {
    abmf::Model<MyAgent> m;
    MyAgent a{1};
    m.addAgent(a);
    ASSERT_EQ(m.getAgents<MyAgent>().size(), 1);
}

TEST(AgentTest, RemovingAgent) {
    abmf::Model<MyAgent> m;
    m.emplaceAgent<MyAgent>(1);
    m.emplaceAgent<MyAgent>(2);
    m.emplaceAgent<MyAgent>(3);

    m.getAgents<MyAgent>()[0].active = false;

    m.removeInactiveAgents();
    ASSERT_EQ(m.getAgents<MyAgent>().size(), 2);
}
