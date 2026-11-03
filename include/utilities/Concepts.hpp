#pragma once

#include <concepts>
#include <functional>
#include <list>
#include <optional>

#include "../space/Point.hpp"

namespace agh {
/** @brief Agent exposing an activity predicate. */
template<typename T>
concept ActiveAgent = requires(T t) {
    { t.isActive() } -> std::convertible_to<bool>;
};

/** @brief Agent exposing an optional integer position. */
template<typename A>
concept Positionable = requires(A a) {
    { a.pos } -> std::same_as<std::optional<Point>&>;
};

/** @brief Agent exposing an optional continuous position. */
template<typename A>
concept RealPositionable = requires(A a) {
    {a.pos} -> std::same_as<std::optional<RealPoint>&>;
};

/** @brief Hashable and equality-comparable edge label. */
template<typename L>
concept Label = requires(L l) {
    std::hash<L>();
    requires std::equality_comparable<L>;
};

/** @brief Hashable and equality-comparable graph node. */
template<typename N>
concept Node = requires(N n) {
    std::hash<N>();
    requires std::equality_comparable<N>;
};

/** @brief Active agent supporting step with state M. */
template<typename A, typename M>
concept Schedulable = requires(A a, M m) {
    a.step(m);
} && ActiveAgent<A>;

/** @brief Agent supporting the optional advance phase with state M. */
template<typename A, typename M>
concept Advanceable = requires(A a, M m) {
  a.advance(m);
};

/** @brief State providing scheduler hooks and a termination predicate. */
template<typename M>
concept SimState = requires(M m) {
    m.beforeStep();
    m.afterStep();
    { m.shouldEnd(int{}) } -> std::convertible_to<bool>;
};
}
