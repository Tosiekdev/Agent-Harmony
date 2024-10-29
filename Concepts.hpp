#pragma once

#include <concepts>
#include <optional>

#include "space/Point.hpp"

namespace abmf {
template<typename A>
concept Positionable = std::same_as<decltype(std::declval<A>().pos), std::optional<Point>>;
}
