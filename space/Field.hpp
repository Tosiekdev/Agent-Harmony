#pragma once

#include "../Concepts.hpp"
#include "Point.hpp"

#include <functional>
#include <optional>
#include <variant>
#include <vector>

namespace abmf {
template<Positionable... Agents>
class Field {
public:
  using AgentT = std::variant<std::reference_wrapper<Agents>...>;
  using OptAgentT = std::optional<AgentT>;
  using GridT = std::vector<std::vector<OptAgentT>>;

  explicit Field(const int pWidth, const int pHeight, const bool torus = false)
    : width(pWidth), height(pHeight), grid(height, std::vector<OptAgentT>(width)), toroidal(torus) {}

  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  bool addAgent(Agent& agent, Point pos);

  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  bool moveAgent(Agent& agent, Point pos);

  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  void removeAgent(Agent& agent);
  void removeAgent(Point pos);

  template<typename Visitor>
  void apply(Visitor&& f);

  template<std::invocable<Point, AgentT&> F>
  void transform(F&& f);

  OptAgentT& getAgent(Point pos);

  [[nodiscard]] bool isEmpty(Point p) const;

  [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;
  [[nodiscard]] std::vector<AgentT> getNeighbors(Point pos, int r, bool moore, bool center);
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

#include "FieldImpl.hpp"
