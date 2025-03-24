#pragma once

#include "../utilities/Concepts.hpp"

#include <variant>

namespace abmf {

template<RealPostionable... Agents> requires (sizeof...(Agents) > 0)
class ContinuousSpace {
public:
    using AgentT = std::variant<Agents*...>;
    using SquareT = std::vector<AgentT>;
    using GridT = std::vector<SquareT>;

private:
    double width;
    double height;
    double discretization;
};
}
