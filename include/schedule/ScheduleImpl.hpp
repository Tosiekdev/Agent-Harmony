#pragma once

#include <chrono>

namespace agh {
template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleOnce(auto& agent, const size_t time, const size_t order) {
    actions.emplace(agent, time, order);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t order,
                                               const size_t interval) {
    actions.emplace(agent, time, order, interval);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
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

    if constexpr ((Advanceable<Agents, M> && ...)) {
        for (auto& event : events) {
            event.advance(model);
        }
    }

    for (auto& event : events) {
        std::visit([&](auto agent) {
            if (agent->isActive() && event.interval) {
                scheduleRepeating(event.agent, epochs + event.interval, event.order, event.interval);
            }
        }, event.agent);
    }

    model.afterStep();
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
size_t Schedule<M, Agents...>::getEpochs() const {
    return epochs;
}

template<SimState M, Schedulable<M> ... Agents> requires (sizeof...(Agents) > 0)
bool Schedule<M, Agents...>::isActive() const {
    return !model.shouldEnd(epochs);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::execute() {
    while (isActive()) {
        step();
    }
}
}
