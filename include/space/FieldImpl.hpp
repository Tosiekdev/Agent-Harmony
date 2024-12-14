#pragma once

#include "../utilities/Utils.hpp"
#include "Field.hpp"

namespace abmf {
template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
bool Field<Agents...>::addAgent(Agent& agent, Point pos) {
    if (getAgent(pos)) return false;
    getAgent(pos) = &agent;
    agent.pos = pos;
    return true;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
bool Field<Agents...>::moveAgent(Agent& agent, Point pos) {
    if (getAgent(pos)) return false;
    if (!agent.pos) {
        return addAgent(agent, pos);
    }
    getAgent(*agent.pos) = std::nullopt;
    getAgent(pos) = &agent;
    agent.pos = pos;
    return true;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void Field<Agents...>::removeAgent(Agent& agent) {
    if (agent.pos) {
        getAgent(*agent.pos) = std::nullopt;
    }
    agent.pos = std::nullopt;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
void Field<Agents...>::removeAgent(const Point pos) {
    if (OptAgentT& square = getAgent(pos)) {
        std::visit([&](auto agent) { removeAgent(*agent); }, *square);
    }
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename Visitor> requires (std::invocable<Visitor, Agents&> || ...)
void Field<Agents...>::apply(Visitor&& f) {
    applyToAll(grid,
               [&](OptAgentT& agent) {
                   if (!agent.has_value()) return;
                   std::visit([&](auto a){std::invoke(std::forward<Visitor>(f), *a);}, *agent);
               }, width, height);
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
template<typename F> requires (std::invocable<F, Point, Agents&> || ...)
void Field<Agents...>::transform(F&& f) {
    transformAll(grid, [&](Point p, OptAgentT& agent) {
        if (!agent.has_value()) return;
        std::visit([&](auto a){std::invoke(std::forward<F>(f), p, *a);}, *agent);
    }, width, height);
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
auto Field<Agents...>::getAgent(const Point pos) -> OptAgentT& {
    return grid[pos.y * width + pos.x];
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
bool Field<Agents...>::isEmpty(Point p) {
    return !getAgent(p).has_value();
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
std::vector<Point> Field<Agents...>::getNeighborhood(Point pos, int r, bool moore, bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
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

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
bool Field<Agents...>::outOfBounds(const Point p) const {
    return p.x < 0 || p.x >= width || p.y < 0 || p.y >= height;
}

template<Positionable ... Agents> requires (sizeof...(Agents) > 0)
Point Field<Agents...>::toToroidal(const Point p) const {
    return convertToToroidal(p, width, height);
}
}
