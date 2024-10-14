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

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::step() {
    if (actions.empty()) {
        return;
    }

    epochs = actions.top().time;

    std::vector<Action<M, Agents...>> events;
    events.reserve(actions.size());
    while (true) {
        if (actions.empty()) {
            break;
        }

        if (actions.top().time > epochs) {
            break;
        }

        events.push_back(actions.top());
        actions.pop();
    }

    for (auto& event : events) {
        event.step();

        if (event.interval) {
            scheduleRepeating(event.agent, event.time, event.priority, event.interval);
        }
    }
}

}
