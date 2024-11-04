#pragma once

#include "../Utils.hpp"
#include "Field.hpp"

namespace abmf {
template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
bool Field<Agents...>::addAgent(Agent& agent, Point pos) {
    if (grid[pos.y][pos.x]) return false;
    grid[pos.y][pos.x] = &agent;
    agent.pos = pos;
    return true;
}

template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
bool Field<Agents...>::moveAgent(Agent& agent, Point pos) {
    if (grid[pos.y][pos.x]) return false;
    if (grid[agent.pos.y][agent.pos.x]) {
        grid[agent.pos.y][agent.pos.x] = std::nullopt;
    }
    grid[pos.y][pos.y] = &agent;
    agent.pos = pos;
    return true;
}

template<Positionable ... Agents>
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void Field<Agents...>::removeAgent(Agent& agent) {
    if (agent.pos) {
        getAgent(*agent.pos) = std::nullopt;
    }
    agent.pos = std::nullopt;
}

template<Positionable ... Agents>
void Field<Agents...>::removeAgent(const Point pos) {
    if (OptAgentT& square = getAgent(pos)) {
        std::visit([&](auto agent) { removeAgent(*agent); }, *square);
    }
}

template<Positionable ... Agents>
template<typename Visitor>
void Field<Agents...>::apply(Visitor&& f) {
    applyToAll(grid,
               [&](OptAgentT& agent) {
                   if (!agent.has_value()) return;
                   std::visit([&](auto a){std::invoke(std::forward<Visitor>(f), *a);}, *agent);
               });
}

template<Positionable ... Agents>
template<std::invocable<Point, std::variant<Agents*...>&> F>
void Field<Agents...>::transform(F&& f) {
    transformAll(grid, [&](Point p, OptAgentT& agent) {
        if (!agent.has_value()) return;
        std::invoke(std::forward<F>(f), p, *agent);
    });
}

template<Positionable ... Agents>
auto Field<Agents...>::getAgent(Point pos) -> OptAgentT& {
    return grid[pos.y][pos.x];
}

template<Positionable ... Agents>
bool Field<Agents...>::isEmpty(Point p) const {
    return grid[p.y][p.x].has_value();
}

template<Positionable ... Agents>
std::vector<Point> Field<Agents...>::getNeighborhood(Point pos, int r, bool moore, bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<Positionable ... Agents>
std::vector<typename Field<Agents...>::AgentT> Field<Agents...>::getNeighbors(
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
                    if (isEmpty(pos)) continue;
                    result.push_back(*getAgent(p));
                }
            }
            else {
                if (isEmpty(pos)) continue;
                result.push_back(*getAgent(p));
            }
        }
    }

    return result;
}

template<Positionable ... Agents>
bool Field<Agents...>::outOfBounds(const Point p) const {
    return p.x < 0 || p.x > width || p.y < 0 || p.y > height;
}

template<Positionable ... Agents>
Point Field<Agents...>::toToroidal(const Point p) const {
    return convertToToroidal(p, width, height);
}
}
