#pragma once

#include "Schedule.hpp"

namespace abmf {

template<typename M, Stepable<M>... Agents>
void Schedule<M, Agents...>::scheduleOnce(auto &agent, const size_t time, const size_t priority) {
    actions.emplace_back(agent, time, priority);
}

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::scheduleRepeating(auto &agent, const size_t time, const size_t priority,
                                               const size_t interval) {
    actions.emplace_back(agent, time, priority, interval);
}

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::scheduleRepeating(auto &agent, const size_t time, const size_t priority) {
    scheduleRepeating(agent, time, priority, 1);
}

}
