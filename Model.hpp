#pragma once

#include <list>
#include <tuple>

#include "AgentBase.hpp"

namespace abmf {

template<ActiveAgent... Agents>
class Model {
public:
    template<ActiveAgent T>
    std::list<T>& getAgents();

    template<ActiveAgent T>
    T& addAgent(const T& agent);

    template<ActiveAgent T, typename... Args>
    T& emplaceAgent(Args&&... args);

    void removeInactiveAgents();

    size_t agentCount();

protected:
    std::tuple<std::list<Agents>...> agents{};
};

}

#include "ModelImpl.hpp"
