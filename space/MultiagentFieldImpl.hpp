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
void MultiagentField<Agents...>::moveAgent(Agent& agent, Point pos) {
    removeAgent(agent);
    addAgent(agent, pos);
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
        agent.pos = std::nullopt;
    }
}

template<Positionable ... Agents>
void MultiagentField<Agents...>::removeAgents(const Point pos) {
    for (auto& agent : getAgents(pos)) {
        std::visit([&](auto a) { a.get().pos = std::nullopt; }, agent);
    }
    getAgents(pos).clear();
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

template<Positionable ... Agents>
template<std::invocable<Point, std::variant<std::reference_wrapper<Agents>...>&> F>
void MultiagentField<Agents...>::transform(F&& f) {
    transformAll(grid, [&](Point p, SquareT& agents) {
        for (auto& agent : agents) {
            std::invoke(std::forward<F>(f), p, agent);
        }
    });
}

template<Positionable ... Agents>
bool MultiagentField<Agents...>::isEmpty(Point p) const {
    return getAgents(p).empty();
}

template<Positionable ... Agents>
size_t MultiagentField<Agents...>::agentCount(Point p) const {
    return getAgents(p).size();
}

template<Positionable ... Agents>
std::vector<Point> MultiagentField<Agents...>::getNeighborhood(Point pos, int r, bool moore, bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<Positionable ... Agents>
bool MultiagentField<Agents...>::outOfBounds(const Point p) const {
    return p.x < 0 || p.x > width || p.y < 0 || p.y > height;
}

template<Positionable ... Agents>
Point MultiagentField<Agents...>::toToroidal(const Point p) const {
    return convertToToroidal(p, width, height);
}
}
