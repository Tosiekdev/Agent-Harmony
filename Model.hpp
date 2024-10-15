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
    T& addAgent(const T& agent);

    template<ActiveAgent T, typename... Args>
    T& emplaceAgent(Args&&... args);

    void removeInactiveAgents();

    size_t agentCount();

protected:
    std::tuple<std::vector<Agents>...> agents{};
};

}

#include "ModelImpl.hpp"
