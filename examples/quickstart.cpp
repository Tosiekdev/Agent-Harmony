#include <AgentHarmony.hpp>

struct Agent {
    std::optional<agh::Point> pos;
    int updates = 0;
    bool isActive() const { return true; }
    template<typename State> void step(State&) { ++updates; }
};
struct State : agh::Model<Agent> {
    void beforeStep() {}
    void afterStep() {}
    bool shouldEnd(std::size_t time) const { return time >= 3; }
};
int main() {
    State model;
    auto& agent = model.emplaceAgent<Agent>();
    agh::Field<Agent> field(4, 2, true);
    field.addAgent(agent, {0, 0});
    agh::Schedule<State, Agent> schedule(model);
    schedule.scheduleRepeating(agent, 1, 0);
    schedule.execute();
    return agent.updates == 3 && model.agentCount() == 1 ? 0 : 1;
}
