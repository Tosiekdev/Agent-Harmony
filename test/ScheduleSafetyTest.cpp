#include <gtest/gtest.h>
#include <functional>
#include <array>
#include <limits>
#include <string>
#include "schedule/Schedule.hpp"

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

TEST(ScheduleSafetyTest, EmptyStepCannotWrapTime) {
    State model;
    Agent a;
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleOnce(a, std::numeric_limits<std::size_t>::max(), 0);
    schedule.step();
    EXPECT_THROW(schedule.step(), std::overflow_error);
    EXPECT_EQ(schedule.getEpochs(), std::numeric_limits<std::size_t>::max());
}

TEST(ScheduleSafetyTest, RepeatingOverflowLeavesBatchAvailable) {
    State model;
    Agent a, b;
    agh::Schedule<State, Agent> schedule(model);
    const auto time = std::numeric_limits<std::size_t>::max();
    schedule.scheduleOnce(a, time, 0);
    schedule.scheduleRepeating(b, time, 1, 1);
    EXPECT_THROW(schedule.step(), std::overflow_error);
    EXPECT_EQ(a.steps, 0);
    EXPECT_EQ(b.steps, 0);
    EXPECT_EQ(schedule.getEpochs(), 0);
    EXPECT_THROW(schedule.step(), std::overflow_error);
    EXPECT_EQ(a.steps, 0);
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
    EXPECT_EQ(skipped.steps, 0);
    EXPECT_EQ(first.advances, 1);
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
