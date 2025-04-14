#pragma once

#include "../utilities/Concepts.hpp"
#include "Point.hpp"

#include <variant>
#include <vector>

namespace agh {
/**
 * Representation of two-dimensional grid. It allows storage of the multiple agents in one cell.
 * @tparam Agents Types of the agents to be stored in the struct. They must meet Positionable requirements.
 */
template<Positionable... Agents> requires (sizeof...(Agents) > 0)
class MultiagentField {
public:
    /**
     * Type of the stored agent.
     */
    using AgentT = std::variant<Agents*...>;

    /**
     * Type of the cell.
     */
    using SquareT = std::vector<AgentT>;

    /**
     * Type of the grid.
     */
    using GridT = std::vector<SquareT>;

    /**
     * Creates empty grid that has specified attributes.
     * @param pWidth Width of the grid.
     * @param pHeight Height of the grid.
     * @param torus Should space wrap.
     */
    explicit MultiagentField(const int pWidth, const int pHeight, const bool torus = false)
        : width(pWidth), height(pHeight), grid(height * width), toroidal(torus) {}

    /**
     * Crates empty grid that has specified attributes. Additionally, it reserves requested space for agents per grid.
     * @param pWidth Width of the grid.
     * @param pHeight Height of the grid.
     * @param reservation Number of agents per grid to reserve.
     * @param torus Should space wrap.
     */
    explicit MultiagentField(const int pWidth, const int pHeight, size_t reservation, const bool torus = false)
        : width(pWidth), height(pHeight), grid(height * width), toroidal(torus) {
        for (auto& cell : grid) {
            cell.reserve(reservation);
        }
    }

    /**
     * Gets all agents present at the given position. Returns empty vector if there is no agent at given position.
     * @param pos Position to get agents from.
     * @return Reference to the specified cell.
     */
    SquareT& getAgents(Point pos);

    /**
     * Gets all agents present at the given position. Returns empty vector if there is no agent at given position.
     * @param pos Position to get agents from.
     * @return Reference to the specified constant cell.
     */
    const SquareT& getAgents(Point pos) const;

    /**
     * Adds agent on the field at specified position.It sets pos attribute of the added agent to the specified position.
     * @tparam Agent Type of the agent we want to add.
     * @param agent Agent to be added to the field.
     * @param pos Position at which we want to place an agent.
     */
    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
    void addAgent(Agent& agent, Point pos);

    /**
     * Moves agent to the specified location. If agent wasn't present on the field it is added. It modifies value of the
     * pos attribute of the agent.
     * @tparam Agent Type of the agent we want to move.
     * @param agent Agent to be moved.
     * @param pos Position we want to move agent to.
     */
    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void moveAgent(Agent& agent, Point pos);

    /**
     * Removes specified agent from the grid. This modifies the value of the pos attribute of the agent.
     * @tparam Agent Type of the agent we want to remove.
     * @param agent Agent to be removed.
     */
    template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...) && std::equality_comparable<Agent>
    void removeAgent(Agent& agent);

    /**
     * Removes all agents at specified grid cell. This modifies the value of the pos attribute of deleted agents.
     * @param pos Position at which we want to remove agents.
     */
    void removeAgents(Point pos);

    /**
     * Calls specified function for every agent on the grid.
     * @tparam F Type of the invoked function.
     * @param f Function to be invoked. It must be invocable with a reference to an agent.
     */
    template<typename F> requires (std::invocable<F, Agents&> || ...)
    void apply(F&& f);

    /**
     * Calls specified function for every pair (coordinates, agent) on the grid.
     * @tparam F Type of the invoked function.
     * @param f Function to be invoked. It must be invocable with an object of type Point and a reference to an agent.
     */
    template<typename F> requires (std::invocable<F, Point, Agents&> || ...)
    void transform(F&& f);

    /**
     * Checks if the cell at the given position is empty.
     * @param p Position of the cell we want to check to see if it is empty.
     * @return Boolean value indicating if the specified cell is empty.
     */
    [[nodiscard]] bool isEmpty(Point p) const;

    /**
     * Get agent count at the given position.
     * @param p Point at which we want to count agents.
     * @return Number of agent at the specified cell.
     */
    [[nodiscard]] size_t agentCount(Point p) const;

    /**
     *
     * @param pos Point which neighborhood we want to get.
     * @param r Radius of the neighborhood.
     * @param moore Flag indicating if we want a Moore (true), or von Neumann (false) neighborhood.
     * @param center If set to true, also the central point will be returned.
     * @return Vector of points neighbouring the specified one.
     */
    [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;

    /**
     * Get agents neighboring (according to the specified criteria) the chosen central point.
     * @param pos Point which neighbors we want to get.
     * @param r Radius of the neighborhood we want to get.
     * @param moore Flag indicating if we want a Moore (true), or von Neumann (false) neighborhood.
     * @param center If set to true, also the central point will be returned.
     * @return Vector of pointers to agents neighbouring with the specified grid cell.
     */
    [[nodiscard]] std::vector<AgentT> getNeighbors(Point pos, int r, bool moore, bool center);

    /**
     * Checks if given point is beyond the grid.
     * @param p Point to be checked.
     * @return Boolean value indicating if given point is out of the bounds of the grid.
     */
    [[nodiscard]] bool outOfBounds(Point p) const;

    /**
     * Maps the given point to the coordinates it would have if the grid were toroidal.
     * @param p Point to be converted.
     * @return Point mapped to the proper coordinates.
     */
    [[nodiscard]] Point toToroidal(Point p) const;

    /**
     * Gets all empty cells of the grid.
     * @return Vector containing the coordinates of grids without an agent.
     */
    [[nodiscard]] std::vector<Point> getEmpty() const;

    /**
     * Gets with of the grid. Equivalent to the maximum x coordinate plus one.
     * @return Width of the grid.
     */
    [[nodiscard]] int getWidth() const { return width; }

    /**
     * Gets height of the grid. Equivalent to the maximum y coordinate plus one.
     * @return Height of the grid.
     */
    [[nodiscard]] int getHeight() const { return height; }

    /**
     * Checks if grid is wrapped (top edge is connected with bottom edge, and left edge is connected with right edge).
     * @return True if grid is representing wrapped space, false otherwise
     */
    [[nodiscard]] bool isToroidal() const { return toroidal; }

private:
    int width;
    int height;
    GridT grid;
    bool toroidal;
};
}

#include "MultiagentFieldImpl.hpp"
