#pragma once

#include "../Utils.hpp"
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

template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
void MultiagentField<Agents...>::removeAgent(Agent& agent) {
    if (agent.pos) {
        std::erase_if(getAgents(*agent.pos), [&](AgentT agentVariant) {
            return std::visit([&](auto a) {
                if constexpr (std::is_same_v<decltype(a), std::reference_wrapper<Agent>>) {
                    return a.get() == agent;
                }
                return false;
            }, agentVariant);
        });
    }
}

template<Positionable ... Agents>
template<typename Visitor>
void MultiagentField<Agents...>::apply(Visitor&& f) {
    applyToAll(grid, [&](SquareT& square) {
        for(auto agent : square) {
            std::visit(std::forward<Visitor>(f), agent);
        }
    });
}
}
