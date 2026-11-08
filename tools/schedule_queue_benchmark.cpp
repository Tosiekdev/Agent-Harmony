#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <string_view>
#include <tuple>
#include <vector>

#include "schedule/Schedule.hpp"

namespace {
constexpr std::size_t eventCount = 262'144;
constexpr std::size_t eventsPerDenseTick = 256;
constexpr std::size_t sparsePairSpacing = 64;

struct Event {
    std::size_t time;
    std::size_t order;
    std::size_t id;
};

struct State {
    std::vector<std::size_t> trace;
    std::size_t steps = 0;

    void beforeStep() { ++steps; }
    void afterStep() {}
    bool shouldEnd(std::size_t) const { return trace.size() >= eventCount; }
};

struct Agent {
    std::size_t id;

    bool isActive() const { return true; }
    void step(State& state) const { state.trace.push_back(id); }
};
}

int main(int argc, char** argv) {
    const std::string_view workload = argc >= 2 ? argv[1] : "";
    const bool dense = workload == "dense";
    const bool emitTrace = argc == 3 && std::string_view(argv[2]) == "--trace";
    if ((!dense && workload != "sparse") || (argc != 2 && !emitTrace)) {
        std::cerr << "Usage: schedule_queue_benchmark dense|sparse [--trace]\n";
        return 1;
    }

    std::vector<Agent> agents;
    std::vector<Event> events;
    agents.reserve(eventCount);
    events.reserve(eventCount);
    for (std::size_t id = 0; id < eventCount; ++id) {
        const auto tick = dense ? id / eventsPerDenseTick : id - id / sparsePairSpacing;
        agents.push_back({id});
        events.push_back({tick + 1, (id / 3) % 8, id});
    }
    std::mt19937 random(42);
    std::shuffle(events.begin(), events.end(), random);

    State state;
    state.trace.reserve(eventCount);
    agh::Schedule<State, Agent> schedule(state);
    const auto expectedSteps = dense ? eventCount / eventsPerDenseTick
                                     : eventCount - (eventCount - 1) / sparsePairSpacing;

    using Clock = std::chrono::steady_clock;
    const auto start = Clock::now();
    for (const auto& event : events) {
        schedule.scheduleOnce(agents[event.id], event.time, event.order);
    }
    const auto enqueued = Clock::now();
    while (state.steps < expectedSteps && state.trace.size() < eventCount) {
        schedule.step();
    }
    const auto drained = Clock::now();

    if (state.steps != expectedSteps || state.trace.size() != eventCount ||
        schedule.getEpochs() != expectedSteps) {
        std::cerr << "Unexpected number of events or steps, or unexpected final tick\n";
        return 1;
    }
    // Stable sorting supplies the insertion-order tie break independently of the scheduler.
    std::stable_sort(events.begin(), events.end(), [](const Event& left, const Event& right) {
        return std::tie(left.time, left.order) < std::tie(right.time, right.order);
    });
    for (std::size_t index = 0; index < eventCount; ++index) {
        if (state.trace[index] != events[index].id) {
            std::cerr << "Unexpected event at trace index " << index << '\n';
            return 1;
        }
    }

    if (emitTrace) {
        for (const auto id : state.trace) std::cout << id << '\n';
    } else {
        using Milliseconds = std::chrono::duration<double, std::milli>;
        std::cout << Milliseconds(enqueued - start).count() << ','
                  << Milliseconds(drained - enqueued).count() << ','
                  << Milliseconds(drained - start).count() << '\n';
    }
    return 0;
}
