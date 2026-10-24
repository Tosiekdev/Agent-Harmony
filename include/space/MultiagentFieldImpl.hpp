#pragma once

#include "../utilities/Utils.hpp"

namespace agh {
template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
auto MultiagentField<Agents...>::cell(Point pos) -> SquareT& {
    return grid[detail::checkedIndex(pos, width, height, toroidal)];
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
auto MultiagentField<Agents...>::getAgents(const Point pos) -> const SquareT& {
    return grid[detail::checkedIndex(pos, width, height, toroidal)];
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
auto MultiagentField<Agents...>::getAgents(const Point pos) const -> const SquareT& {
    return grid[detail::checkedIndex(pos, width, height, toroidal)];
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent>
void MultiagentField<Agents...>::addAgent(Agent& agent, Point pos) requires (std::is_same_v<Agent, Agents> || ...) {
    detail::requireStructuralMutation(traversing);
    if (agent.pos) throw std::logic_error("Agent is already positioned; remove it before adding");
    pos = detail::checkedPosition(pos, width, height, toroidal);
    cell(pos).push_back(&agent);
    agent.pos = pos;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent>
void MultiagentField<Agents...>::moveAgent(Agent& agent, Point pos) requires (std::is_same_v<Agent, Agents> || ...) {
    detail::requireStructuralMutation(traversing);
    pos = detail::checkedPosition(pos, width, height, toroidal);
    if (!agent.pos) { addAgent(agent, pos); return; }
    if (outOfBounds(*agent.pos))
        throw std::logic_error("Agent does not belong to this space");
    auto& source = cell(*agent.pos);
    auto found = std::find(source.begin(), source.end(), AgentT{&agent});
    if (found == source.end()) throw std::logic_error("Agent does not belong to this space");
    auto& destination = cell(pos);
    if (&source != &destination) {
        destination.push_back(&agent);
        source.erase(found);
    }
    agent.pos = pos;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent>
void MultiagentField<Agents...>::removeAgent(Agent& agent) requires (std::is_same_v<Agent, Agents> || ...) {
    detail::requireStructuralMutation(traversing);
    if (!agent.pos) return;
    if (outOfBounds(*agent.pos)) return;
    auto& source = cell(*agent.pos);
    auto found = std::find(source.begin(), source.end(), AgentT{&agent});
    if (found == source.end()) return;
    source.erase(found);
    agent.pos.reset();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
void MultiagentField<Agents...>::removeAgents(const Point pos) {
    detail::requireStructuralMutation(traversing);
    for (auto& agent : cell(pos)) {
        std::visit([&](auto a) { a->pos = std::nullopt; }, agent);
    }
    cell(pos).clear();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F>
void MultiagentField<Agents...>::apply(F&& f) requires (std::invocable<F, Agents&> && ...) {
    detail::TraversalGuard guard(traversing);
    applyToAll(grid,
               [&](SquareT& square) {
                   for (auto agent : square) {
                       std::visit([&](auto a) { std::invoke(std::forward<F>(f), *a); }, agent);
                   }
               },
               width,
               height);
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F>
void MultiagentField<Agents...>::transform(F&& f) requires (std::invocable<F, Point, Agents&> && ...) {
    detail::TraversalGuard guard(traversing);
    transformAll(grid,
                 [&](Point p, SquareT& agents) {
                     for (auto& agent : agents) {
                         std::visit([&](auto a) { std::invoke(std::forward<F>(f), p, *a); }, agent);
                     }
                 },
                 width,
                 height);
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
bool MultiagentField<Agents...>::isEmpty(const Point p) const {
    return getAgents(p).empty();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
size_t MultiagentField<Agents...>::agentCount(const Point p) const {
    return getAgents(p).size();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<Point> MultiagentField<Agents...>::getNeighborhood(Point pos, int r, bool moore, bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<typename MultiagentField<Agents...>::AgentT> MultiagentField<Agents...>::getNeighbors(
    const Point pos, const int r, const bool moore, const bool center) {
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
std::vector<Point> MultiagentField<Agents...>::getEmpty() const {
    std::vector<Point> result;
    result.reserve(grid.size() / 2);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (isEmpty({x, y})) {
                result.push_back({x, y});
            }
        }
    }

    return result;
}
}
