#pragma once

#include "Field.hpp"

namespace abmf {
template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
bool Field<Agents...>::addAgent(Agent& agent, Point pos) {
    if (!grid[pos.y][pos.x]) return false;
    grid[pos.y][pos.x] = agent;
    agent.pos = pos;
    return true;
}

template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
bool Field<Agents...>::moveAgent(Agent& agent, Point pos) {
    if (!grid[pos.y][pos.x]) return false;
    if (grid[agent.pos.y][agent.pos.x]) {
        grid[agent.pos.y][agent.pos.x] = std::nullopt;
    }
    grid[pos.y][pos.y] = agent;
    agent.pos = pos;
    return true;
}
}
