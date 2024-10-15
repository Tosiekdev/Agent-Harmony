#pragma once

#include <algorithm>
#include <iostream>

#include "Model.hpp"

namespace abmf {
template<ActiveAgent... Agents>
template<ActiveAgent T>
std::vector<T>& Model<Agents...>::getAgents() {
    return std::get<std::vector<T>>(agents);
}

template<ActiveAgent... Agents>
template<ActiveAgent T>
T& Model<Agents...>::addAgent(const T& agent) {
    getAgents<T>().push_back(agent);
    return getAgents<T>().back();
}

template<ActiveAgent ... Agents>
template<ActiveAgent T, typename... Args>
T& Model<Agents...>::emplaceAgent(Args&&... args) {
    getAgents<T>().emplace_back(std::forward<Args>(args)...);
    return getAgents<T>().back();
}

template<ActiveAgent ... Agents>
void Model<Agents...>::removeInactiveAgents() {
    ([&] {
        auto& agents = getAgents<Agents>();
        agents.erase(std::partition(agents.begin(), agents.end(), [](Agents a) { return a.isActive(); }),
                     agents.end());
    }(), ...);
}

template<ActiveAgent ... Agents>
size_t Model<Agents...>::agentCount() {
    return std::apply([](auto... v) {
                          return (v.size() + ...);
                      },
                      agents);
}
}
