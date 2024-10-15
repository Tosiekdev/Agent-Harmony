#pragma once

#include "Schedule.hpp"

namespace abmf {
template<typename M, Stepable<M>... Agents>
void Schedule<M, Agents...>::scheduleOnce(auto& agent, const size_t time, const size_t priority) {
    actions.emplace(agent, time, priority);
}

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t priority,
                                               const size_t interval) {
    actions.emplace(agent, time, priority, interval);
}

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t priority) {
    scheduleRepeating(agent, time, priority, 1);
}

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::step() {
    model.step();
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
        event.step(model);

        if (event.interval) {
            scheduleRepeating(event.agent, epochs + event.interval, event.priority, event.interval);
        }
    }
}

template<typename M, Stepable<M> ... Agents>
size_t Schedule<M, Agents...>::getEpochs() const {
    return epochs;
}

template<typename M, Stepable<M> ... Agents>
void Schedule<M, Agents...>::execute() {
    while (!model.shouldEnd()) {
        step();
    }
}
}
