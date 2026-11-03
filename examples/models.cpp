#include <model/Model.hpp>

struct Agent {
    int value;
    explicit Agent(int initial) : value(initial) {}
    Agent(const Agent&) = delete;
    bool isActive() const { return true; }
};
int main() {
    agh::Model<Agent> model;
    auto& agent = model.emplaceAgent<Agent>(3);
    for (auto& item : model.getAgents<Agent>()) ++item.value;
    const auto& state = model;
    return state.agentCount() == 1 && state.agentCount<Agent>() == 1 && agent.value == 4 ? 0 : 1;
}
