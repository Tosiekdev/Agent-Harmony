#pragma once

#include "Model.hpp"

namespace abmf {

template<typename... Agents>
template<typename T>
std::vector<T> Model<Agents...>::getAgents() {
    return std::get<std::vector<T>>(agents);
}

}
