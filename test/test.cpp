#include <gtest/gtest.h>

#include "../Model.hpp"
#include "../Schedule.hpp"

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

struct MyModel : abmf::Model<MyAgent> {
    void step() {
        std::cout << agentCount() << std::endl;
    }
};

TEST(AgentsTest, AddingAgent) {
    MyModel m;
    m.emplaceAgent<MyAgent>(0);
    ASSERT_EQ(m.agentCount(), 1);
}

TEST(AgentTest, RemovingAgent) {
    MyModel m;
    m.emplaceAgent<MyAgent>(1);
    m.emplaceAgent<MyAgent>(2);
    m.emplaceAgent<MyAgent>(3);

    m.getAgents<MyAgent>()[0].active = false;

    m.removeInactiveAgents();
    ASSERT_EQ(m.agentCount(), 2);
}

TEST(ActionTest, CreateAction) {
    MyAgent a{1};
    MyModel m;
    abmf::Action<MyModel, MyAgent> action(a, 1, 1);
    action.step(m);
}
