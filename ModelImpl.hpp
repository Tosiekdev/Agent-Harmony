#pragma once

#include "Model.hpp"

template<typename ... Agents>
template<typename T>
std::vector<T> Model<Agents...>::getAgents() {
    return std::get<T>(agents);
}
