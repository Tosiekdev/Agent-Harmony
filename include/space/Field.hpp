#pragma once

#include "../utilities/Concepts.hpp"
#include "Point.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace agh {
/**
 * Representation of two-dimensional grid. It allows storage of the one agent per cell.
 * @tparam Agents Types of the agents to be stored in the struct. They must meet Positionable requirements.
 */
template<Positionable... Agents> requires (sizeof...(Agents) > 0)
class Field {
public:
  /**
   * Type of the agent stored at given cell.
   */
  using AgentT = std::variant<Agents*...>;

  /**
   * Type of the cell.
   */
  using OptAgentT = std::optional<AgentT>;


  /**
   * Type of the grid.
   */
  using GridT = std::vector<OptAgentT>;

  /**
   * Creates empty grid that has specified attributes.
   * @param pWidth Width of the grid.
   * @param pHeight Height of the grid.
   * @param torus Should space wrap.
   */
  explicit Field(const int pWidth, const int pHeight, const bool torus = false)
    : width(pWidth), height(pHeight), grid(height * width), toroidal(torus) {}

  /**
   * Gets agent at the given position. Returns std::nullopt when at given position there is no agent.
   * @param pos Position to check agent at.
   * @return Reference to the specified cell.
   */
  OptAgentT& getAgent(Point pos);

  /**
   * Gets agent at the given position. Returns std::nullopt when at given position there is no agent.
   * @param pos Position to check agent at.
   * @return Reference to the specified constant cell.
   */
  const OptAgentT& getAgent(Point pos) const;

  /**
   * Adds agent on the field at specified position only if there is no other agent at that position. It sets pos
   * attribute of the added agent to the specified position.
   * @tparam Agent Type of the agent we want to add.
   * @param agent Agent to be added to the field.
   * @param pos Position at which we want to place an agent.
   * @return Boolean value indicating if an agent was successfully added.
   */
  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  bool addAgent(Agent& agent, Point pos);

  /**
   * Moves agent to the specified location if it is empty. If agent wasn't present on the field it is added. It modifies
   * value of the pos attribute of the agent.
   * @tparam Agent Type of the agent we want to move.
   * @param agent Agent to be moved.
   * @param pos Position we want to move agent to.
   * @return Boolean value indicating if an agent was successfully moved.
   */
  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  bool moveAgent(Agent& agent, Point pos);

  /**
   * Removes specified agent from the grid. This modifies the value of the pos attribute of the agent.
   * @tparam Agent Type of the agent we want to remove.
   * @param agent Agent to be removed.
   */
  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  void removeAgent(Agent& agent);

  /**
   * Removes agent at specified grid cell, leaving it empty. This modifies the value of the pos attribute of the agent.
   * @param pos Position at which we want to remove an agent.
   */
  void removeAgent(Point pos);

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
   * Returns all points that are neighboring (according to the specified criteria) the chosen central point.
   * @param pos Point which neighborhood we want to get.
   * @param r Radius of the neighborhood.
   * @param moore Flag indicating if we want a Moore (true), or von Neumann (false) neighborhood.
   * @param center If set to true, also the central point will be returned.
   * @return Vector of points neighbouring the specified one.
   */
  [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r=1, bool moore=true, bool center=false) const;

  /**
   * Get agents neighboring (according to the specified criteria) the chosen central point.
   * @param pos Point which neighbors we want to get.
   * @param r Radius of the neighborhood we want to get.
   * @param moore Flag indicating if we want a Moore (true), or von Neumann (false) neighborhood.
   * @param center If set to true, also agent at the central point will be returned.
   * @return Vector of pointers to agents neighbouring with the specified grid cell.
   */
  [[nodiscard]] std::vector<AgentT> getNeighbors(Point pos, int r=1, bool moore=true, bool center=false);

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
   * @return True if grid is representing wrapped space, false otherwise.
   */
  [[nodiscard]] bool isToroidal() const { return toroidal; }

private:
  int width;
  int height;
  GridT grid;
  bool toroidal;
};
}

#include "FieldImpl.hpp"
