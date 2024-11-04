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

TEST(ValueLayerTest, ForEach) {}

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
    field.addAgent(agent, {1, 1});
    field.apply([](auto& a) { a.value += 2; });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, Transform) {
    using FieldT = abmf::Field<MyAgent>;
    MyAgent agent;
    FieldT field(2, 2);
    field.addAgent(agent, {1, 1});
    field.transform([](abmf::Point p, FieldT::AgentT a) {
        std::visit([&](auto ag) { ag->value += p.x + p.y; }, a);
    });
    EXPECT_EQ(agent.value, 2);
}

TEST(FieldTest, RemoveAgent) {
    using FieldT = abmf::Field<MyAgent>;
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

TEST(MultiagentFieldTest, AddAgent) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent;
    MyAgent agent2;
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    EXPECT_EQ(field.getAgents({0,0}).size(), 2);
}

TEST(MultiagentFieldTest, Apply) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent;
    MyAgent agent2;
    Field field(2, 2);
    field.addAgent(agent, {0, 0});
    field.addAgent(agent2, {0, 0});
    field.apply([](auto a) { a->value += 2; });
    EXPECT_EQ(agent.value, 2);
    EXPECT_EQ(agent2.value, 2);
}

TEST(MultiagentFieldTest, Transform) {
    using Field = abmf::MultiagentField<MyAgent>;
    MyAgent agent;
    MyAgent agent2;
    MyAgent agent3;
    Field field(2, 2);
    field.addAgent(agent, {0, 1});
    field.addAgent(agent2, {1, 1});
    field.addAgent(agent3, {1, 1});
    field.transform([](abmf::Point p, Field::AgentT a) {
        std::visit([&](auto ag) { ag->value += p.x + p.y; }, a);
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

namespace test::ma_field::ngh {
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
