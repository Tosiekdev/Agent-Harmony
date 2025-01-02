#pragma once

#include "../utilities/Utils.hpp"
#include "MultiagentField.hpp"

namespace abmf {
template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
auto MultiagentField<Agents...>::getAgents(Point pos) -> SquareT& {
    return grid[pos.y][pos.x];
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
auto MultiagentField<Agents...>::getAgents(Point pos) const -> const SquareT& {
    return grid[pos.y][pos.x];
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void MultiagentField<Agents...>::addAgent(Agent& agent, Point pos) {
    agent.pos = pos;
    getAgents(pos).push_back(&agent);
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
void MultiagentField<Agents...>::moveAgent(Agent& agent, Point pos) {
    removeAgent(agent);
    addAgent(agent, pos);
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
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

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
void MultiagentField<Agents...>::removeAgents(const Point pos) {
    for (auto& agent : getAgents(pos)) {
        std::visit([&](auto a) { a->pos = std::nullopt; }, agent);
    }
    getAgents(pos).clear();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F> requires (std::invocable<F, Agents&> || ...)
void MultiagentField<Agents...>::apply(F&& f) {
    applyToAll(grid,
               [&](SquareT& square) {
                   for (auto agent : square) {
                       std::visit([&](auto a) { std::invoke(std::forward<F>(f), *a); }, agent);
                   }
               });
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F> requires (std::invocable<F, Point, Agents&> || ...)
void MultiagentField<Agents...>::transform(F&& f) {
    transformAll(grid,
                 [&](Point p, SquareT& agents) {
                     for (auto& agent : agents) {
                         std::visit([&](auto a) { std::invoke(std::forward<F>(f), p, *a); }, agent);
                     }
                 });
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
bool MultiagentField<Agents...>::isEmpty(Point p) const {
    return getAgents(p).empty();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
size_t MultiagentField<Agents...>::agentCount(Point p) const {
    return getAgents(p).size();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<Point> MultiagentField<Agents...>::getNeighborhood(Point pos, int r, bool moore, bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<typename MultiagentField<Agents...>::AgentT> MultiagentField<Agents...>::getNeighbors(
    const Point pos, int r, const bool moore, const bool center) {
    auto f = [&](const Point p, std::vector<AgentT>& result) {
        for (auto& agent : getAgents(p)) {
            result.push_back(agent);
        }
    };
    return visitNeighbors<MultiagentField, AgentT, decltype(f)>(*this,
                                                                pos,
                                                                r,
                                                                moore,
                                                                center,
                                                                std::forward<decltype(f)>(f));
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
bool MultiagentField<Agents...>::outOfBounds(const Point p) const {
    return p.x < 0 || p.x >= width || p.y < 0 || p.y >= height;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
Point MultiagentField<Agents...>::toToroidal(const Point p) const {
    return convertToToroidal(p, width, height);
}

template<Positionable... Agents> requires (sizeof...(Agents) > 0)
std::vector<Point> MultiagentField<Agents...>::getEmpty() {
    std::vector<Point> result;
    result.reserve(width*height / 2);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < height; ++x) {
            if (isEmpty({x,y})) {
                result.push_back({x, y});
            }
        }
    }

    return result;
}
}
