#pragma once

#include <deque>
#include <tuple>

#include "AgentBase.hpp"

namespace abmf {
template<ActiveAgent... Agents>
class Model {
public:
    template<ActiveAgent T>
    std::deque<T>& getAgents();

    template<ActiveAgent T>
    T& addAgent(const T& agent);

    template<ActiveAgent T, typename... Args>
    T& emplaceAgent(Args&&... args);

    size_t agentCount();

protected:
    std::tuple<std::deque<Agents>...> agents{};
};
}

#include "ModelImpl.hpp"
