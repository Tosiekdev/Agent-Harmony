#include <gtest/gtest.h>
#include <functional>
#include <array>
#include <limits>
#include <string>
#include "schedule/Schedule.hpp"
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

struct MyModel : agh::Model<MyAgent> {
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

struct SecondModel : agh::Model<AdvAgent> {
    void beforeStep() {}

    void afterStep() {}

    [[nodiscard]] bool shouldEnd(const int epochs) const {
        return epochs >= 10;
    }
};

TEST(ActionTest, CreateAction) {
    MyAgent a{1};
    MyModel m;
    agh::action::Action<MyModel, MyAgent> action(a, 1, 1);
    action.step(m);
    EXPECT_EQ(a.run, 1);
}

TEST(SchedulerTest, ScheduleEvent) {
    MyModel model;
    agh::Schedule<MyModel, MyAgent> schedule(model);
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
    agh::Schedule<SecondModel, AdvAgent> schedule(model);
    auto& agent = model.emplaceAgent<AdvAgent>(1, 2);
    schedule.scheduleRepeating(agent, 1, 1, 1);
    schedule.execute();
    EXPECT_EQ(agent.adv, 10);
}
}

namespace test::schedule_safety {

namespace {
struct State {
    std::function<void()> before = [] {};
    std::function<void()> after = [] {};
    void beforeStep() { before(); }
    void afterStep() { after(); }
    bool shouldEnd(std::size_t epochs) const { return epochs >= 10; }
};
struct Agent {
    bool active = true;
    int steps = 0;
    std::function<void()> action = [] {};
    bool isActive() const { return active; }
    void step(State&) { ++steps; action(); }
};
struct AdvancingAgent : Agent {
    int advances = 0;
    void advance(State&) { ++advances; }
};
}

TEST(ScheduleSafetyTest, RejectPastAndZeroIntervalWithoutChangingQueue) {
    State model;
    Agent a;
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleOnce(a, 10, 0);
    schedule.step();
    EXPECT_EQ(schedule.getEpochs(), 10);
    EXPECT_THROW(schedule.scheduleOnce(a, 5, 0), std::invalid_argument);
    EXPECT_THROW(schedule.scheduleRepeating(a, 9, 0, 1), std::invalid_argument);
    EXPECT_THROW(schedule.scheduleRepeating(a, 10, 0, 0), std::invalid_argument);
    schedule.scheduleOnce(a, 10, 0);
    schedule.step();
    EXPECT_EQ(a.steps, 2);
    EXPECT_EQ(schedule.getEpochs(), 10);
}

TEST(ScheduleSafetyTest, EmptyStepWrapsTime) {
    State model;
    Agent a;
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleOnce(a, std::numeric_limits<std::size_t>::max(), 0);
    schedule.step();
    EXPECT_NO_THROW(schedule.step());
    EXPECT_EQ(schedule.getEpochs(), 0);
}

TEST(ScheduleSafetyTest, RepeatingSuccessorWrapsTime) {
    State model;
    Agent a;
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleRepeating(a, std::numeric_limits<std::size_t>::max(), 0, 1);
    schedule.step();
    EXPECT_EQ(a.steps, 1);
    EXPECT_EQ(schedule.getEpochs(), std::numeric_limits<std::size_t>::max());
    EXPECT_THROW(schedule.scheduleOnce(a, 0, 0), std::invalid_argument);
    schedule.step();
    EXPECT_EQ(a.steps, 2);
    EXPECT_EQ(schedule.getEpochs(), 0);
}

TEST(ScheduleSafetyTest, EqualTimeAndPriorityAreFIFO) {
    State model;
    std::vector<int> order;
    std::array<Agent, 6> agents{};
    agh::Schedule<State, Agent> schedule(model);
    for (int i = 0; i < 6; ++i) {
        agents[i].action = [&, i] { order.push_back(i); };
        schedule.scheduleOnce(agents[i], 2, 1);
    }
    schedule.step();
    EXPECT_EQ(order, (std::vector<int>{0, 1, 2, 3, 4, 5}));
}

TEST(ScheduleSafetyTest, HookSeesNewTimeAndNewEventsWaitForNextCall) {
    State model;
    Agent first, fromHook, fromAgent;
    agh::Schedule<State, Agent> schedule(model);
    std::vector<std::string> trace;
    bool added = false;
    model.before = [&] {
        EXPECT_EQ(schedule.getEpochs(), 5);
        trace.push_back("before");
        if (!added) { schedule.scheduleOnce(fromHook, 5, 0); added = true; }
    };
    model.after = [&] { trace.push_back("after"); };
    first.action = [&] { trace.push_back("first"); schedule.scheduleOnce(fromAgent, 5, 0); };
    fromHook.action = [&] { trace.push_back("hook-event"); };
    fromAgent.action = [&] { trace.push_back("agent-event"); };
    schedule.scheduleOnce(first, 5, 0);
    schedule.step();
    EXPECT_EQ(trace, (std::vector<std::string>{"before", "first", "after"}));
    trace.clear();
    schedule.step();
    EXPECT_EQ(trace, (std::vector<std::string>{"before", "hook-event", "agent-event", "after"}));
}

TEST(ScheduleSafetyTest, MixedAdvanceAndDeactivationPreserveSeparatePhases) {
    State model;
    AdvancingAgent first;
    Agent second, skipped;
    agh::Schedule<State, Agent, AdvancingAgent> schedule(model);
    first.action = [&] { skipped.active = false; first.active = false; EXPECT_EQ(first.advances, 0); };
    second.action = [&] { EXPECT_EQ(first.advances, 0); };
    schedule.scheduleRepeating(first, 1, 0);
    schedule.scheduleOnce(second, 1, 1);
    schedule.scheduleOnce(skipped, 1, 2);
    schedule.step();
    EXPECT_EQ(first.steps, 1);
    EXPECT_EQ(second.steps, 1);
    EXPECT_EQ(skipped.steps, 1);
    EXPECT_EQ(first.advances, 0);
    schedule.step();
    EXPECT_EQ(first.steps, 1);
    EXPECT_EQ(schedule.getEpochs(), 2);
}

TEST(ScheduleSafetyTest, EmptyStepsStillRunHooksAndExecuteUsesShouldEnd) {
    State model;
    int before = 0, after = 0;
    model.before = [&] { ++before; };
    model.after = [&] { ++after; };
    agh::Schedule<State, Agent> schedule(model);
    schedule.execute();
    EXPECT_EQ(schedule.getEpochs(), 10);
    EXPECT_EQ(before, 10);
    EXPECT_EQ(after, 10);
}

TEST(ScheduleSafetyTest, InactiveSelectedEventsExecuteButDoNotRepeat) {
    State model;
    Agent dormant;
    dormant.active = false;
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleRepeating(dormant, 1, 0, 1);
    schedule.step();
    EXPECT_EQ(dormant.steps, 1);
    schedule.step();
    EXPECT_EQ(dormant.steps, 1);
    EXPECT_EQ(schedule.getEpochs(), 2);
}

template<typename T>
concept CanAdvanceAction = requires(T& action, State& state) { action.advance(state); };
static_assert(CanAdvanceAction<agh::action::Action<State, AdvancingAgent>>);
static_assert(!CanAdvanceAction<agh::action::Action<State, Agent, AdvancingAgent>>);

TEST(ScheduleSafetyTest, AllAdvanceCallsFollowAllSteps) {
    struct PhasedAgent : Agent {
        std::function<void()> advancement = [] {};
        void advance(State&) { advancement(); }
    };
    State state;
    PhasedAgent first, second;
    std::vector<int> trace;
    first.action = [&] { trace.push_back(1); first.active = false; };
    second.action = [&] { trace.push_back(2); };
    first.advancement = [&] { trace.push_back(3); };
    second.advancement = [&] { trace.push_back(4); };
    agh::Schedule<State, PhasedAgent> schedule(state);
    schedule.scheduleOnce(first, 1, 0);
    schedule.scheduleOnce(second, 1, 1);
    schedule.step();
    EXPECT_EQ(trace, (std::vector<int>{1, 2, 3, 4}));
}

}
