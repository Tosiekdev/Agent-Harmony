#include <gtest/gtest.h>

#include <optional>

#include "../Model.hpp"
#include "../Schedule.hpp"
#include "../space/Field.hpp"
#include "../space/MultiagentField.hpp"
#include "../space/ValueLayer.hpp"

struct MyAgent {
    int id;
    bool active{true};
    int run{};
    int value{};
    std::optional<abmf::Point> pos;

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

TEST(ValueLayerTest, Transform) {
    abmf::RealValueLayer layer(2, 2, false, 1.);
    layer.transform([](abmf::Point p, double& value) { value += p.x + p.y; });
    layer.swap();
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(layer.get({i, j}), i+j+1.);
        }
    }
}

TEST(ValueLayerTest, Apply) {
    abmf::RealValueLayer layer(2, 2, false, 1.);
    std::vector<double> values;
    layer.apply([&values](double value) mutable { values.push_back(value); });
    EXPECT_EQ(values.size(), 2*2);
}

TEST(ValueLayerTest, ForEach) {

}

TEST(ValueLayerTest, GetNeighborhood) {
    abmf::RealValueLayer layer(3, 3, false, 1.);
    auto moore = layer.getNeighborhood({1, 1}, 1, true, false);
    auto mooreCenter = layer.getNeighborhood({1, 1}, 1, true, true);

    auto vonNeumann = layer.getNeighborhood({1, 1}, 1, false, false);
    auto vonNeumannCenter = layer.getNeighborhood({1, 1}, 1, false, true);

    EXPECT_EQ(moore.size(), 8);
    EXPECT_EQ(mooreCenter.size(), 9);
    EXPECT_EQ(vonNeumann.size(), 4);
    EXPECT_EQ(vonNeumannCenter.size(), 5);
}

TEST(FieldTest, Apply) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1,1});
    field.apply([](auto& a){a.get().value += 2;});
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, RemoveAgent) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1,1});
    EXPECT_TRUE(field.getAgent({1,1}).has_value());
    field.removeAgent({1,1});
    EXPECT_FALSE(field.getAgent({1,1}).has_value());
    field.addAgent(agent, {1,1});
    EXPECT_TRUE(field.getAgent({1,1}).has_value());
    field.removeAgent(agent);
    EXPECT_FALSE(field.getAgent({1,1}).has_value());
}

TEST(MultiagentFieldTest, AddAgent) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent;
    MyAgent agent2;
    Field field(2,2);
    field.addAgent(agent, {0,0});
    field.addAgent(agent2, {0,0});
    EXPECT_EQ(field.getAgents({0,0}).size(), 2);
}
