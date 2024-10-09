#pragma once

#include <tuple>
#include <vector>

namespace abmf {

template<typename... Agents>
class Model {
public:
    template<typename T>
    std::vector<T> getAgents();

private:
    std::tuple<std::vector<Agents>...> agents;
};

}

#include "ModelImpl.hpp"
