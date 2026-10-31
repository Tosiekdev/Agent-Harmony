#include <schedule/Schedule.hpp>
#include <vector>

struct State {
    std::vector<int> trace;
    void beforeStep() {}
    void afterStep() {}
    bool shouldEnd(std::size_t time) const { return time >= 1; }
};
struct Agent {
    int id;
    bool isActive() const { return true; }
    void step(State& state) { state.trace.push_back(id); }
    void advance(State& state) { state.trace.push_back(id + 10); }
};
int main() {
    State model;
    Agent first{1}, second{2};
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleOnce(first, 1, 0);
    schedule.scheduleOnce(second, 1, 0);
    schedule.execute();
    return model.trace == std::vector<int>{1, 2, 11, 12} ? 0 : 1;
}
