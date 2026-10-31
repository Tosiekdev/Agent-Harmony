#pragma once

#include "../utilities/Utils.hpp"

#include <algorithm>
#include <cassert>
#include <limits>

namespace agh {
template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void ContinuousSpace<Agents...>::addAgent(Agent& agent, RealPoint pos) {
    requireStructuralMutation(traversing);
    if (agent.pos) throw std::logic_error("Agent is already positioned; remove it before adding");
    pos = checkedPosition(pos, width, height, toroidal);
    getCell(pos).push_back(&agent);
    agent.pos = pos;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
void ContinuousSpace<Agents...>::moveAgent(Agent& agent, RealPoint pos) {
    requireStructuralMutation(traversing);
    pos = checkedPosition(pos, width, height, toroidal);
    if (agent.pos && (!std::isfinite(agent.pos->x) || !std::isfinite(agent.pos->y) || agent.pos->x < 0 || agent.pos->y < 0 || agent.pos->x >= width || agent.pos->y >= height)) return;
    removeAgent(agent);
    addAgent(agent, pos);
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
void ContinuousSpace<Agents...>::removeAgent(Agent& agent) {
    requireStructuralMutation(traversing);
    if (!agent.pos || (!std::isfinite(agent.pos->x) || !std::isfinite(agent.pos->y) || agent.pos->x < 0 || agent.pos->y < 0 || agent.pos->x >= width || agent.pos->y >= height)) return;
    std::erase_if(getCell(*agent.pos), [&](const AgentT& stored) {
        return std::visit([&](auto pointer) {
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(*pointer)>, Agent>)
                return *pointer == agent;
            else return false;
        }, stored);
    });
    agent.pos.reset();
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F> requires (std::invocable<F, Agents&> && ...)
void ContinuousSpace<Agents...>::apply(F&& f) {
    TraversalGuard guard(traversing);
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
size_t ContinuousSpace<Agents...>::agentCount(RealPoint p) const {
    p = checkedPosition(p, width, height, toroidal);
    return std::ranges::count_if(getCell(p),
                                 [&](AgentT agentPtr) {
                                     return std::visit([&](auto agent) { return agent->pos == p; }, agentPtr);
                                 });
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<typename ContinuousSpace<Agents...>::AgentT> ContinuousSpace<Agents...>::getNeighbors(RealPoint pos,
    const float r, const bool euclidean, const bool center) {
    validateRadius(r);
    pos = checkedPosition(pos, width, height, toroidal);
    auto candidates = [&](double coordinate, double extent, std::size_t count) {
        std::vector<std::size_t> buckets;
        auto append = [&](double low, double high) {
            const std::size_t first = std::min(count - 1, static_cast<std::size_t>(std::floor(low / discretization)));
            const std::size_t last = std::min(count - 1, static_cast<std::size_t>(std::floor(high / discretization)));
            for (std::size_t i = first; i <= last; ++i) buckets.push_back(i);
        };
        const double low = coordinate - r, high = coordinate + r;
        if (!toroidal) append(std::max(0.0, low), std::min(extent, high));
        else if (static_cast<double>(r) >= extent / 2) append(0, extent);
        else if (low < 0) { append(0, high); append(extent + low, extent); }
        else if (high >= extent) { append(low, extent); append(0, high - extent); }
        else append(low, high);
        std::sort(buckets.begin(), buckets.end());
        buckets.erase(std::unique(buckets.begin(), buckets.end()), buckets.end());
        return buckets;
    };
    const auto xs = candidates(pos.x, width, cols);
    const auto ys = candidates(pos.y, height, rows);
    std::vector<AgentT> neighbors;
    for (std::size_t y : ys) for (std::size_t x : xs) {
        for (auto agent : getCell(Point{x, y})) {
            const RealPoint location = std::visit(Pos, agent);
            if (!center && location == pos) continue;
            double dx = std::abs(static_cast<double>(location.x) - pos.x);
            double dy = std::abs(static_cast<double>(location.y) - pos.y);
            if (toroidal) {
                dx = std::min(dx, static_cast<double>(width) - dx);
                dy = std::min(dy, static_cast<double>(height) - dy);
            }
            const double distance = euclidean ? std::hypot(dx, dy) : std::max(dx, dy);
            if (distance <= r) neighbors.push_back(agent);
        }
    }
    return neighbors;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
[[nodiscard]] RealPoint ContinuousSpace<Agents...>::toToroidal(const RealPoint p) const {
    return checkedPosition(p, width, height, true);
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
Point ContinuousSpace<Agents...>::discretize(const RealPoint point) const {
    return {
        std::min(cols - 1, static_cast<std::size_t>(std::floor(static_cast<double>(point.x) / discretization))),
        std::min(rows - 1, static_cast<std::size_t>(std::floor(static_cast<double>(point.y) / discretization)))
    };
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const RealPoint point) -> SquareT& {
    return getCell(discretize(checkedPosition(point, width, height, toroidal)));
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const RealPoint point) const -> const SquareT& {
    return getCell(discretize(checkedPosition(point, width, height, toroidal)));
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const Point point) -> SquareT& {
    return grid[checkedIndex(point, cols, rows)];
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const Point point) const -> const SquareT& {
    return grid[checkedIndex(point, cols, rows)];
}

}
