#include <gtest/gtest.h>

#include "../Model.hpp"
#include "../Schedule.hpp"
#include "../space/ValueLayer.hpp"

struct MyAgent {
    int id;
    bool active{true};
    int run{};

    [[nodiscard]] bool isActive() const {
        return active;
    }

    template<typename T>
    void step(T& model) {
        run += 1;
    }
};

struct MyModel : abmf::Model<MyAgent> {
    explicit MyModel() = default;

    void beforeStep() {
        run += 1;
    }

    void afterStep() {}

    [[nodiscard]] bool shouldEnd(const int epochs) const {
        return epochs >= 10;
    }

    int run{};
};

struct Simulation {
    explicit Simulation() : schedule(model) {}
    MyModel model;
    abmf::Schedule<MyModel, MyAgent> schedule;

    void init() {
        schedule.scheduleRepeating(model.emplaceAgent<MyAgent>(0), 1, 1, 1);
        schedule.scheduleRepeating(model.emplaceAgent<MyAgent>(1), 1, 1, 1);
    }

    void start() {
        schedule.execute();
    }
};

TEST(AgentsTest, AddingAgent) {
    MyModel m;
    m.emplaceAgent<MyAgent>(0);
    ASSERT_EQ(m.agentCount(), 1);
}

TEST(ActionTest, CreateAction) {
    MyAgent a{1};
    MyModel m;
    abmf::Action<MyModel, MyAgent> action(a, 1, 1);
    action.step(m);
}

TEST(SchedulerTest, ScheduleEvent) {
    Simulation s;
    s.init();
    s.start();
    EXPECT_EQ(s.model.run, 10);
    for (const auto& agent : s.model.getAgents<MyAgent>()) {
        EXPECT_EQ(agent.run, 10);
    }
    EXPECT_EQ(s.schedule.getEpochs(), 10);
}

TEST(ValueLayerTest, ApplyUnary) {
    abmf::IntValueLayer layer(2, 2, false);
    layer.apply([](int& value) { value += 2; });
    layer.swap();
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), 2);
        }
    }
}
