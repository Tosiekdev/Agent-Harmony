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
    getAgents(pos).push_back(&agent);
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
        std::erase_if(getAgents(*agent.pos),
                      [&](AgentT agentVariant) {
                          return std::visit([&](auto a) {
                                                if constexpr (std::is_same_v<
                                                    std::remove_reference_t<decltype(*a)>, Agent>) {
                                                    return *a == agent;
                                                }
                                                return false;
                                            },
                                            agentVariant);
                      });
        agent.pos = std::nullopt;
    }
}

template<Positionable ... Agents>
void MultiagentField<Agents...>::removeAgents(const Point pos) {
    for (auto& agent : getAgents(pos)) {
        std::visit([&](auto a) { a->pos = std::nullopt; }, agent);
    }
    getAgents(pos).clear();
}

template<Positionable ... Agents>
template<typename F> requires (std::invocable<F, Agents&> || ...)
void MultiagentField<Agents...>::apply(F&& f) {
    applyToAll(grid,
               [&](SquareT& square) {
                   for (auto agent : square) {
                       std::visit([&](auto a) { std::invoke(std::forward<F>(f), *a); }, agent);
                   }
               });
}

template<Positionable ... Agents>
template<typename F> requires (std::invocable<F, Point, Agents&> || ...)
void MultiagentField<Agents...>::transform(F&& f) {
    transformAll(grid,
                 [&](Point p, SquareT& agents) {
                     for (auto& agent : agents) {
                         std::visit([&](auto a) { std::invoke(std::forward<F>(f), p, *a); }, agent);
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
std::vector<typename MultiagentField<Agents...>::AgentT> MultiagentField<Agents...>::getNeighbors(
    const Point pos, int r, const bool moore, const bool center) {
    std::vector<AgentT> result;
    if (moore) {
        result.reserve((2 * r + 1) * (2 * r + 1));
    }
    else {
        result.reserve(r * r + (r + 1) * (r + 1));
    }
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (!moore && std::abs(dx) + std::abs(dy) > r) continue;

            Point p = {pos.x + dx, pos.y + dy};

            if (p == pos && !center) continue;

            if (outOfBounds(p)) {
                if (isToroidal()) {
                    p = toToroidal(p);
                    for (auto& agent : getAgents(p)) {
                        result.push_back(agent);
                    }
                }
            }
            else {
                for (auto& agent : getAgents(p)) {
                    result.push_back(agent);
                }
            }
        }
    }

    return result;
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
