#pragma once

#include <tuple>
#include <vector>

#include "AgentBase.hpp"

namespace abmf {

template<ActiveAgent... Agents>
class Model {
public:
    template<ActiveAgent T>
    std::vector<T>& getAgents();

    template<ActiveAgent T>
    void addAgent(const T& agent);

    template<ActiveAgent T, typename... Args>
    void emplaceAgent(Args&&... args);

    void removeInactiveAgents();

private:
    std::tuple<std::vector<Agents>...> agents{};
};

}

#include "ModelImpl.hpp"
