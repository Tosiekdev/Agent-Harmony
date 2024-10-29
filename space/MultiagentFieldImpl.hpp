#pragma once

#include "MultiagentField.hpp"

namespace abmf {
template<Positionable ... Agents>
auto MultiagentField<Agents...>::getAgents(Point pos) -> SquareT& {
    return grid[pos.y][pos.x];
}

template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void MultiagentField<Agents...>::addAgent(Agent& agent, Point pos) {
    agent.pos = pos;
    getAgents(pos).push_back(agent);
}
}
