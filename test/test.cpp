#include <gtest/gtest.h>

#include "../Model.hpp"
#include "../Schedule.hpp"

struct MyAgent {
    int id;
    bool active{true};
    int run{};

    bool isActive() const {
        return active;
    }

    template<typename T>
    void step(T &model) {
        run += 1;
    }
};

struct MyModel : abmf::Model<MyAgent> {
    explicit MyModel() : schedule(*this) {}

    void init() {
        schedule.scheduleRepeating(emplaceAgent<MyAgent>(0), 1, 1, 1);
        schedule.scheduleRepeating(emplaceAgent<MyAgent>(1), 1, 1, 1);
    }

    void step() {
        run += 1;
    }

    bool shouldEnd() const {
        return schedule.getEpochs() >= 10;
    }

    abmf::Schedule<MyModel, MyAgent> schedule;
    int run{};
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

    m.getAgents<MyAgent>().front().active = false;

    m.removeInactiveAgents();
    ASSERT_EQ(m.agentCount(), 2);
}

TEST(ActionTest, CreateAction) {
    MyAgent a{1};
    MyModel m;
    abmf::Action<MyModel, MyAgent> action(a, 1, 1);
    action.step(m);
}

TEST(SchedulerTest, ScheduleEvent) {
    MyModel m;
    m.init();
    m.schedule.execute();
    EXPECT_EQ(m.run, 10);
    for (const auto &agent : m.getAgents<MyAgent>()) {
        EXPECT_EQ(agent.run, 10);
    }
    EXPECT_EQ(m.schedule.getEpochs(), 10);
}
