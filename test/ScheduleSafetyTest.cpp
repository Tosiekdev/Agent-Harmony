#include <gtest/gtest.h>
#include <functional>
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
