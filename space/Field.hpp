#pragma once

#include "Point.hpp"

#include <concepts>
#include <functional>
#include <optional>
#include <variant>
#include <vector>

namespace abmf {
template<typename A>
concept Positionable = requires(A a) {
  { a.pos } -> std::same_as<Point>;
};

template<Positionable... Agents>
class Field {
public:
  using GridT = std::vector<std::vector<std::optional<std::variant<std::reference_wrapper<Agents>...>>>>;
  explicit Field(const int pWidth, const int pHeight) : width(pWidth), height(pHeight), grid(height, GridT(width)) {}

  template<Positionable Agent> requires (std::is_same_v<Agent, Agents> || ...)
  bool addAgent(Agent agent, Point pos);

private:
  int width;
  int height;
  GridT grid;
};
}

#include "FieldImpl.hpp"
