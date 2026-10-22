#pragma once

#include <algorithm>

namespace agh {
template<ActiveAgent... Agents> requires (sizeof...(Agents) > 0)
template<ActiveAgent T>
std::ranges::subrange<typename std::deque<T>::iterator> Model<Agents...>::getAgents() {
    auto& storage = std::get<std::deque<T>>(agents);
    return {storage.begin(), storage.end()};
}

template<ActiveAgent... Agents> requires (sizeof...(Agents) > 0)
template<ActiveAgent T>
std::ranges::subrange<typename std::deque<T>::const_iterator> Model<Agents...>::getAgents() const {
    const auto& storage = std::get<std::deque<T>>(agents);
    return {storage.begin(), storage.end()};
}

template<ActiveAgent... Agents> requires (sizeof...(Agents) > 0)
template<ActiveAgent T>
T& Model<Agents...>::addAgent(const T& agent) {
    std::get<std::deque<T>>(agents).push_back(agent);
    return std::get<std::deque<T>>(agents).back();
}

template<ActiveAgent ... Agents> requires (sizeof...(Agents) > 0)
template<ActiveAgent T, typename... Args>
T& Model<Agents...>::emplaceAgent(Args&&... args) {
    std::get<std::deque<T>>(agents).emplace_back(std::forward<Args>(args)...);
    return std::get<std::deque<T>>(agents).back();
}

template<ActiveAgent ... Agents> requires (sizeof...(Agents) > 0)
size_t Model<Agents...>::agentCount() const {
    return std::apply([](const auto&... v) {
                          return (v.size() + ...);
                      },
                      agents);
}

template<ActiveAgent ... Agents> requires (sizeof...(Agents) > 0)
template<ActiveAgent T>
size_t Model<Agents...>::agentCount() const {
    return getAgents<T>().size();
}
}
