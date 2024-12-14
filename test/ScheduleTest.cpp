#include <gtest/gtest.h>

#include "../include/model/Model.hpp"
#include "../include/schedule/Schedule.hpp"

namespace test::schedule {
struct MyAgent {
    explicit MyAgent(const int pId) : id(pId) {}
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

    bool operator==(const MyAgent& rhs) const {
        return id == rhs.id;
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

struct AdvAgent {
    bool active{true};
    int run{};
    int adv{};

    [[nodiscard]] bool isActive() const {
        return active;
    }

    template<typename T>
    void step(T& model) {
        run += 1;
    }

    template<typename T>
    void advance(T& model) {
        adv += 1;
    }
};

struct SecondModel : abmf::Model<AdvAgent> {
    void beforeStep() {}

    void afterStep() {}

    [[nodiscard]] bool shouldEnd(const int epochs) const {
        return epochs >= 10;
    }
};

TEST(ActionTest, CreateAction) {
    MyAgent a{1};
    MyModel m;
    abmf::Action<MyModel, MyAgent> action(a, 1, 1);
    action.step(m);
    EXPECT_EQ(a.run, 1);
}

TEST(SchedulerTest, ScheduleEvent) {
    MyModel model;
    abmf::Schedule<MyModel, MyAgent> schedule(model);
    schedule.scheduleRepeating(model.emplaceAgent<MyAgent>(0), 1, 1, 1);
    schedule.scheduleRepeating(model.emplaceAgent<MyAgent>(1), 1, 1, 1);
    schedule.execute();
    EXPECT_EQ(model.run, 10);
    for (const auto& agent : model.getAgents<MyAgent>()) {
        EXPECT_EQ(agent.run, 10);
    }
    EXPECT_EQ(schedule.getEpochs(), 10);
}

TEST(SchedulerTest, AgentWithAdvance) {
    SecondModel model;
    abmf::Schedule<SecondModel, AdvAgent> schedule(model);
    auto& agent = model.emplaceAgent<AdvAgent>(1, 2);
    schedule.scheduleRepeating(agent, 1, 1, 1);
    schedule.execute();
    EXPECT_EQ(agent.adv, 10);
}
}
