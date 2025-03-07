#pragma once

#include "../utilities/Concepts.hpp"
#include "Point.hpp"

#include <variant>
#include <vector>

namespace abmf {
template<Positionable... Agents> requires (sizeof...(Agents) > 0)
class MultiagentField {
public:
    using AgentT = std::variant<Agents*...>;
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
    const SquareT& getAgents(Point pos) const;

    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
    void addAgent(Agent& agent, Point pos);

    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void moveAgent(Agent& agent, Point pos);

    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void removeAgent(Agent& agent);
    void removeAgents(Point pos);

    template<typename F> requires (std::invocable<F, Agents&> || ...)
    void apply(F&& f);

    template<typename F> requires (std::invocable<F, Point, Agents&> || ...)
    void transform(F&& f);

    [[nodiscard]] bool isEmpty(Point p) const;
    [[nodiscard]] size_t agentCount(Point p) const;

    [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;
    [[nodiscard]] std::vector<AgentT> getNeighbors(Point pos, int r, bool moore, bool center);
    [[nodiscard]] bool outOfBounds(Point p) const;
    [[nodiscard]] Point toToroidal(Point p) const;
    [[nodiscard]] std::vector<Point> getEmpty() const;

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
