#pragma once

#include "../utilities/Utils.hpp"

#include <algorithm>

namespace abmf {
template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void ContinuousSpace<Agents...>::addAgent(Agent& agent, const RealPoint pos) {
    getCell(pos).push_back(&agent);
    agent.pos = pos;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
void ContinuousSpace<Agents...>::moveAgent(Agent& agent, RealPoint pos) {
    removeAgent(agent);
    addAgent(agent, pos);
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
void ContinuousSpace<Agents...>::removeAgent(Agent& agent) {
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

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F> requires (std::invocable<F, Agents&> || ...)
void ContinuousSpace<Agents...>::apply(F&& f) {
    applyToAll(grid,
               [&](SquareT& square) {
                   for (auto agent : square) {
                       std::visit([&](auto a) { std::invoke(std::forward<F>(f), *a); }, agent);
                   }
               },
               cols,
               rows);
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
size_t ContinuousSpace<Agents...>::agentCount(const RealPoint p) const {
    return std::ranges::count_if(getCell(p),
                                 [&](AgentT agentPtr) {
                                     std::visit([&](auto agent) { agent.pos == p; }, agentPtr);
                                 });
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<typename ContinuousSpace<Agents...>::AgentT> ContinuousSpace<Agents...>::getNeighbors(const RealPoint pos,
    const float r, const bool euclidean, const bool center) {
    const int radius = static_cast<int>(std::floor(r / discretization));
    const auto [pX, pY] = dicretize(pos);

    std::vector<AgentT> neighbors;
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            const int x = pX + j;
            const int y = pY + i;

            // lewy górny

            // prawy górny
            {
                if (l2(pos, {x * discretization, y * discretization}) <= r) {
                    auto& cell = getCell(Point{x, y});
                    if (euclidean) {
                        std::ranges::for_each(cell, [&](AgentT agent) {
                            if (l2(pos, std::visit(Pos, agent)) <= r) {
                                neighbors.push_back(agent);
                            }
                        });
                    } else {
                        neighbors.insert(neighbors.end(), cell.begin(), cell.end());
                    }
                }
            }

            // lewy dolny

            // prawy dolny
        }
    }

    return neighbors;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
[[nodiscard]] RealPoint ContinuousSpace<Agents...>::toToroidal(const RealPoint p) const {
    return {std::fmod(p.x, width), std::fmod(p.y, height)};
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
Point ContinuousSpace<Agents...>::dicretize(const RealPoint point) const {
    return {
        static_cast<int>(std::ceil(point.x / discretization)),
        static_cast<int>(std::ceil(point.y / discretization))
    };
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const RealPoint point) -> SquareT& {
    return getCell(dicretize(point));
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const Point point) -> SquareT& {
    return grid[point.y * rows + point.x];
}
}
