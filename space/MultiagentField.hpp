#pragma once

#include "../Concepts.hpp"
#include "Point.hpp"

#include <variant>
#include <vector>

namespace abmf {
template<Positionable... Agents>
class MultiagentField {
public:
    using AgentT = std::variant<std::reference_wrapper<Agents>...>;
    using SquareT = std::vector<AgentT>;
    using GridT = std::vector<std::vector<SquareT>>;

    explicit MultiagentField(const int pWidth, const int pHeight, const bool torus = false)
        : width(pWidth), height(pHeight), grid(height, std::vector<SquareT>(width)), toroidal(torus) {}

    explicit MultiagentField(const int pWidth, const int pHeight, size_t reservation, const bool torus = false)
        : width(pWidth), height(pHeight), grid(height, std::vector<SquareT>(width)), toroidal(torus) {
        for (auto& row : grid) {
            for (auto& square : row) {
                square.reserve(reservation);
            }
        }
    }

    SquareT& getAgents(Point pos);

    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
    void addAgent(Agent& agent, Point pos);

    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void removeAgent(Agent& agent);

    template<typename Visitor>
    void apply(Visitor&& f);

    [[nodiscard]] bool isEmpty(Point p) const;
    [[nodiscard]] size_t agentCount(Point p) const;

    [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;
    [[nodiscard]] bool outOfBounds(Point p) const;
    [[nodiscard]] Point toToroidal(Point p) const;

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] bool isToroidal() const { return toroidal; }

private:
    int width;
    int height;
    GridT grid;
    bool toroidal;
};
}

#include "MultiagentFieldImpl.hpp"
