#pragma once

#include <concepts>
#include <optional>

#include "space/Point.hpp"

namespace abmf {
template<typename A>
concept Positionable = requires(A a) {
    { a.pos } -> std::convertible_to<std::optional<Point>>;
};
}
