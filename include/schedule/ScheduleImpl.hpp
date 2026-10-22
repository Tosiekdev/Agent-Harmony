#pragma once

#include <limits>
#include <stdexcept>

namespace agh {
template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleOnce(auto& agent, const size_t time, const size_t order) {
    if (time < epochs) throw std::invalid_argument("Cannot schedule an event in the past");
    actions.emplace(agent, time, order);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t order,
                                               const size_t interval) {
    if (time < epochs || interval == 0)
        throw std::invalid_argument("Repeating events require a current/future time and positive interval");
    actions.emplace(agent, time, order, interval);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::step() {
    if (actions.empty() && epochs == std::numeric_limits<size_t>::max())
        throw std::overflow_error("Schedule time exhausted");
    model.beforeStep();
    if (actions.empty()) {
        epochs += 1;
        model.afterStep();
        return;
    }

    const size_t batchTime = actions.top().time;

    std::vector<ActionItem> events;
    events.reserve(actions.size());
    while (true) {
        if (actions.empty()) {
            break;
        }

        if (actions.top().time > batchTime) {
            break;
        }

        if (!actions.top().isActive()) {
            actions.pop();
            continue;
        }

        if (actions.top().interval > std::numeric_limits<size_t>::max() - batchTime) {
            for (const auto& event : events) actions.push(event);
            throw std::overflow_error("Repeating event time exhausted");
        }
        events.push_back(actions.top());
        actions.pop();
    }

    epochs = batchTime;
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
