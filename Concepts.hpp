#pragma once

#include <concepts>
#include <functional>
#include <optional>

#include "space/Point.hpp"

namespace abmf {
template<typename A>
concept Positionable = requires(A a) {
    { a.pos } -> std::convertible_to<std::optional<Point>>;
};

template<typename L>
concept Label = requires(L l) {
    { std::hash<L>() } -> std::same_as<size_t>;
    requires std::equality_comparable<L>;
};

template<typename N>
concept Node = requires(N n) {
    { std::hash<N*>() } -> std::same_as<size_t>;
    requires std::equality_comparable<N>;
};
}
