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
void ContinuousSpace<Agents...>::moveAgent(Agent& agent, const Point pos) {
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
    return std::ranges::count_if(getCell(p), [&](AgentT agentPtr) {
        std::visit([&](auto agent){ agent.pos == p; }, agentPtr);
    });
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
    const auto [x, y] = dicretize(point);
    return grid[y * rows + x];
}
}
