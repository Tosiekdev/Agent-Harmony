#pragma once

#include "Model.hpp"

namespace abmf {

template<Agent... Agents>
template<Agent T>
std::vector<T>& Model<Agents...>::getAgents() {
    return std::get<std::vector<T>>(agents);
}

template<Agent... Agents>
template<Agent T>
void Model<Agents...>::addAgent(const T& agent) {
    getAgents<T>().push_back(agent);
}

template<Agent... Agents>
template<Agent T, typename... Args>
void Model<Agents...>::emplaceAgent(Args&&... args) {
    getAgents<T>().emplace_back(std::forward<Args>(args)...);
}

}
