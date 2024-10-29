#pragma once

#include "Point.hpp"

#include <concepts>
#include <optional>
#include <variant>
#include <vector>

namespace abmf {
template<typename A>
concept Positionable = requires(A a) {
    { a.pos } -> std::same_as<std::optional<Point>>;
};

template<Positionable... Agents>
class MultiagentField {
public:
    using AgentT = std::variant<std::reference_wrapper<Agents>...>;
    using SquareT = std::vector<AgentT>;
    using GridT = std::vector<std::vector<SquareT>>;

private:
    GridT grid;
};
}

#include "MultiagentFieldImpl.hpp"
