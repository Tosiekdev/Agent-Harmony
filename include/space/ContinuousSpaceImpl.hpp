#pragma once

namespace abmf {
template<RealPositionable ... Agents> requires (sizeof...(Agents) > 0)
template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
void ContinuousSpace<Agents...>::addAgent(Agent& agent, const RealPoint pos) {
    getCell(pos).push_back(&agent);
    agent.pos = pos;
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
