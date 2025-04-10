#pragma once

#include "../utilities/Concepts.hpp"

#include <cmath>
#include <variant>

namespace abmf {
/**
 * Representation of two-dimensional continuous field.
 * @tparam Agents Types of the agents to be stored in the struct. They must meet RealPositionable requirements.
 */
template<RealPositionable... Agents> requires (sizeof...(Agents) > 0)
class ContinuousSpace {
public:
    using AgentT = std::variant<Agents*...>;
    using SquareT = std::vector<AgentT>;
    using GridT = std::vector<SquareT>;

    /**
     * Creates empty space that has specified attributes.
     * @param pWidth Width of the space.
     * @param pHeight Height of the space.
     * @param dx Discretization of the space.
     * @param torus Should space wrap.
     */
    explicit ContinuousSpace(const float pWidth, const float pHeight, const float dx, const bool torus = false)
        : width(pWidth), height(pHeight), discretization(dx)
          , rows(static_cast<int>(std::ceil(pHeight / dx)))
          , cols(static_cast<int>(std::ceil(pWidth / dx)))
          , grid(rows * cols)
          , toroidal(torus) {}

    /**
     * Adds agent on the field at specified position. It sets pos attribute of the added agent to the specified
     * position.
     * @tparam Agent Type of the agent we want to add.
     * @param agent Agent to be added to the field.
     * @param pos Position at which we want to place an agent.
     */
    template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
    void addAgent(Agent& agent, RealPoint pos);

    /**
     * Moves agent to the specified location. If agent wasn't present on the field it is added. It modifies value of the
     * pos attribute of the agent.
     * @tparam Agent Type of the agent we want to move.
     * @param agent Agent to be moved.
     * @param pos Position we want to move agent to.
     */
    template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void moveAgent(Agent& agent, RealPoint pos);

    /**
     * Removes specified agent from the field. This modifies the value of the pos attribute of the agent.
     * @tparam Agent Type of the agent we want to remove.
     * @param agent Agent to be removed.
     */
    template<RealPositionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void removeAgent(Agent& agent);

    /**
     * Calls specified function for every agent on the field.
     * @tparam F Type of the invoked function.
     * @param f Function to be invoked. It must be invocable with a reference to an agent.
     */
    template<typename F> requires (std::invocable<F, Agents&> || ...)
    void apply(F&& f);

    /**
     * Get agent count at the given position.
     * @param p Point at which we want to count agents.
     * @return Number of agent at the specified cell.
     */
    [[nodiscard]] size_t agentCount(RealPoint p) const;

    /**
     * Get agents neighboring (according to the specified criteria) the chosen central point.
     * @param pos Point which neighbors we want to get.
     * @param r Radius of the neighborhood we want to get.
     * @param euclidean Flag indicating if we calculate distance using Euclidean distance or Chebyshev distance.
     * @param center If set to true, also the central point will be returned.
     * @return Vector of pointers to agents neighbouring with the specified grid cell.
     */
    [[nodiscard]] std::vector<AgentT> getNeighbors(RealPoint pos, float r, bool euclidean = false, bool center = false);

    /**
     * Maps the given point to the coordinates it would have if the grid were toroidal.
     * @param p Point to be converted.
     * @return Point mapped to the proper coordinates.
     */
    [[nodiscard]] RealPoint toToroidal(RealPoint p) const;

    /**
     * Gets with of the grid. Equivalent to the first x coordinate which is out of bounds.
     * @return Width of the grid.
     */
    [[nodiscard]] float getWidth() const { return width; }

    /**
     * Gets height of the grid. Equivalent to the first y coordinate which is out of bound.
     * @return Height of the grid.
     */
    [[nodiscard]] float getHeight() const { return height; }

    /**
     * Checks if grid is wrapped (top edge is connected with bottom edge, and left edge is connected with right edge).
     * @return True if grid is representing wrapped space, false otherwise
     */
    [[nodiscard]] bool isToroidal() const { return toroidal; }

private:
    float width;
    float height;
    float discretization;
    int rows;
    int cols;
    GridT grid;
    bool toroidal;

    [[nodiscard]] Point discretize(RealPoint point) const;
    [[nodiscard]] SquareT& getCell(RealPoint point);
    [[nodiscard]] SquareT& getCell(Point point);
    [[nodiscard]] bool inRadius(Point point, float radius, RealPoint center) const;
};
}

#include "ContinuousSpaceImpl.hpp"
