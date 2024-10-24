#pragma once

#include "Schedule.hpp"

namespace abmf {
template<SimState M, Schedulable<M>... Agents>
void Schedule<M, Agents...>::scheduleOnce(auto& agent, const size_t time, const size_t priority) {
    actions.emplace(agent, time, priority);
}

template<SimState M, Schedulable<M>... Agents>
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t priority,
                                               const size_t interval) {
    actions.emplace(agent, time, priority, interval);
}

template<SimState M, Schedulable<M> ... Agents>
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t priority) {
    scheduleRepeating(agent, time, priority, 1);
}

template<SimState M, Schedulable<M>... Agents>
void Schedule<M, Agents...>::step() {
    model.beforeStep();
    if (actions.empty()) {
        epochs += 1;
        model.afterStep();
        return;
    }

    epochs = actions.top().time;

    std::vector<ActionItem> events;
    events.reserve(actions.size());
    while (true) {
        if (actions.empty()) {
            break;
        }

        if (actions.top().time > epochs) {
            break;
        }

        if (!actions.top().isActive()) {
            actions.pop();
            continue;
        }

        events.push_back(actions.top());
        actions.pop();
    }

    for (auto& event : events) {
        event.step(model);
    }

    for (auto& event : events) {
        std::visit([&](auto agent) {
            if (agent.get().isActive() && event.interval) {
                scheduleRepeating(event.agent, epochs + event.interval, event.priority, event.interval);
            }
        }, event.agent);
    }

    model.afterStep();
}

template<SimState M, Schedulable<M>... Agents>
size_t Schedule<M, Agents...>::getEpochs() const {
    return epochs;
}

template<SimState M, Schedulable<M>... Agents>
void Schedule<M, Agents...>::execute() {
    while (!model.shouldEnd(epochs)) {
        step();
    }
}
}
