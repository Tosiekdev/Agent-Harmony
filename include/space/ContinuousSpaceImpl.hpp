#pragma once

#include "../utilities/Utils.hpp"

#include <algorithm>
#include <cassert>
#include <limits>

namespace agh {
template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void ContinuousSpace<Agents...>::addAgent(Agent& agent, RealPoint pos) {
    detail::requireStructuralMutation(traversing);
    if (agent.pos) throw std::logic_error("Agent is already positioned; remove it before adding");
    pos = detail::checkedPosition(pos, width, height, toroidal);
    getCell(pos).push_back(&agent);
    agent.pos = pos;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void ContinuousSpace<Agents...>::moveAgent(Agent& agent, RealPoint pos) {
    detail::requireStructuralMutation(traversing);
    pos = detail::checkedPosition(pos, width, height, toroidal);
    if (!agent.pos) { addAgent(agent, pos); return; }
    if (agent.pos->x < 0 || agent.pos->y < 0 || agent.pos->x >= width || agent.pos->y >= height)
        throw std::logic_error("Agent does not belong to this space");
    auto& source = getCell(*agent.pos);
    auto found = std::find(source.begin(), source.end(), AgentT{&agent});
    if (found == source.end()) throw std::logic_error("Agent does not belong to this space");
    auto& destination = getCell(pos);
    if (&source != &destination) {
        destination.push_back(&agent);
        source.erase(found);
    }
    agent.pos = pos;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void ContinuousSpace<Agents...>::removeAgent(Agent& agent) {
    detail::requireStructuralMutation(traversing);
    if (!agent.pos) return;
    if (agent.pos->x < 0 || agent.pos->y < 0 || agent.pos->x >= width || agent.pos->y >= height) return;
    auto& source = getCell(*agent.pos);
    auto found = std::find(source.begin(), source.end(), AgentT{&agent});
    if (found == source.end()) return;
    source.erase(found);
    agent.pos.reset();
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F> requires (std::invocable<F, Agents&> && ...)
void ContinuousSpace<Agents...>::apply(F&& f) {
    detail::TraversalGuard guard(traversing);
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
    p = detail::checkedPosition(p, width, height, toroidal);
    return std::ranges::count_if(getCell(p),
                                 [&](AgentT agentPtr) {
                                     return std::visit([&](auto agent) { return agent->pos == p; }, agentPtr);
                                 });
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<typename ContinuousSpace<Agents...>::AgentT> ContinuousSpace<Agents...>::getNeighbors(RealPoint pos,
    const float r, const bool euclidean, const bool center) {
    detail::validateRadius(r);
    pos = detail::checkedPosition(pos, width, height, toroidal);
    auto candidates = [&](double coordinate, double extent, int count) {
        auto interval = [&](double low, double high) {
            const int first = std::min(count - 1, static_cast<int>(std::floor(low / discretization)));
            const int last = std::min(count - 1, static_cast<int>(std::floor(high / discretization)));
            return std::pair{first, last + 1};
        };
        detail::NeighborhoodRanges ranges{};
        const double low = coordinate - r, high = coordinate + r;
        if (!toroidal) ranges[0] = interval(std::max(0.0, low), std::min(extent, high));
        else if (static_cast<double>(r) >= extent / 2) ranges[0] = {0, count};
        else if (low < 0) {
            ranges[0] = interval(0, high);
            ranges[1] = interval(extent + low, extent);
        } else if (high >= extent) {
            ranges[0] = interval(0, high - extent);
            ranges[1] = interval(low, extent);
        } else ranges[0] = interval(low, high);
        // Coarse or partial buckets can occur in both wrapped intervals.
        // Merge them while retaining the existing sorted bucket traversal.
        if (ranges[1].second && ranges[0].second >= ranges[1].first) {
            ranges[0].second = std::max(ranges[0].second, ranges[1].second);
            ranges[1] = {0, 0};
        }
        return ranges;
    };
    const auto xs = candidates(pos.x, width, cols);
    const auto ys = candidates(pos.y, height, rows);
    const double radiusSquared = static_cast<double>(r) * r;
    std::vector<AgentT> neighbors;
    for (const auto& [yBegin, yEnd] : ys) for (int y = yBegin; y < yEnd; ++y)
    for (const auto& [xBegin, xEnd] : xs) for (int x = xBegin; x < xEnd; ++x) {
        for (auto agent : getCell(Point{x, y})) {
            const RealPoint location = std::visit(Pos, agent);
            if (!center && location == pos) continue;
            double dx = std::abs(static_cast<double>(location.x) - pos.x);
            double dy = std::abs(static_cast<double>(location.y) - pos.y);
            if (toroidal) {
                dx = std::min(dx, static_cast<double>(width) - dx);
                dy = std::min(dy, static_cast<double>(height) - dy);
            }
            bool inside;
            if (euclidean) {
                const double distanceSquared = dx * dx + dy * dy;
                const double roundingBand = 8 * std::numeric_limits<double>::epsilon()
                                            * std::max(distanceSquared, radiusSquared);
                // Preserve the old hypot decision near a rounded circle boundary.
                // This band selects the calculation; it does not enlarge the radius.
                inside = std::abs(distanceSquared - radiusSquared) <= roundingBand
                             ? std::hypot(dx, dy) <= r : distanceSquared <= radiusSquared;
            } else inside = std::max(dx, dy) <= r;
            if (inside) neighbors.push_back(agent);
        }
    }
    return neighbors;
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
[[nodiscard]] RealPoint ContinuousSpace<Agents...>::toToroidal(const RealPoint p) const {
    return detail::checkedPosition(p, width, height, true);
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
Point ContinuousSpace<Agents...>::discretize(const RealPoint point) const {
    return {
        std::min(cols - 1, static_cast<int>(std::floor(static_cast<double>(point.x) / discretization))),
        std::min(rows - 1, static_cast<int>(std::floor(static_cast<double>(point.y) / discretization)))
    };
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const RealPoint point) -> SquareT& {
    return getCell(discretize(detail::checkedPosition(point, width, height, toroidal)));
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const RealPoint point) const -> const SquareT& {
    return getCell(discretize(detail::checkedPosition(point, width, height, toroidal)));
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const Point point) -> SquareT& {
    return grid[detail::checkedIndex(point, cols, rows)];
}

template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
auto ContinuousSpace<Agents...>::getCell(const Point point) const -> const SquareT& {
    return grid[detail::checkedIndex(point, cols, rows)];
}

}
