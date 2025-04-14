#pragma once

#include <concepts>
#include <functional>
#include <optional>

#include "../space/Point.hpp"

namespace agh {
template<typename T>
concept ActiveAgent = requires(T t) {
    { t.isActive() } -> std::convertible_to<bool>;
};

template<typename A>
concept Positionable = requires(A a) {
    { a.pos } -> std::same_as<std::optional<Point>&>;
};

template<typename A>
concept RealPositionable = requires(A a) {
    {a.pos} -> std::same_as<std::optional<RealPoint>&>;
};

template<typename L>
concept Label = requires(L l) {
    std::hash<L>();
    requires std::equality_comparable<L>;
};

template<typename N>
concept Node = requires(N n) {
    std::hash<N*>();
    requires std::equality_comparable<N>;
};

template<typename A, typename M>
concept Schedulable = requires(A a, M m) {
    a.step(m);
} && ActiveAgent<A>;

template<typename A, typename M>
concept Advanceable = requires(A a, M m) {
  a.advance(m);
};

template<typename M>
concept SimState = requires(M m) {
    m.beforeStep();
    m.afterStep();
    { m.shouldEnd(int{}) } -> std::convertible_to<bool>;
};
}
