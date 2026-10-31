#pragma once

#include <limits>
#include <stdexcept>

namespace agh {
template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleOnce(auto& agent, const size_t time, const size_t order) {
    if (time < epochs) throw std::invalid_argument("Cannot schedule an event in the past");
    enqueue(agent, time, order, 0);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t order,
                                               const size_t interval) {
    if (time < epochs || interval == 0)
        throw std::invalid_argument("Repeating events require a current/future time and positive interval");
    enqueue(agent, time, order, interval);
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::enqueue(auto& agent, size_t time, size_t order, size_t interval) {
    if (nextSequence == std::numeric_limits<size_t>::max()) throw std::overflow_error("Event sequence exhausted");
    actions.emplace(agent, time, order, interval, nextSequence);
    ++nextSequence;
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::step() {
    if (actions.empty()) {
        ++epochs;
        model.beforeStep();
        model.afterStep();
        return;
    }
    epochs = actions.top().time;
    std::vector<ActionItem> events;
    events.reserve(actions.size());
    while (!actions.empty() && actions.top().time == epochs) {
        events.push_back(actions.top());
        actions.pop();
    }
    model.beforeStep();
    for (auto& event : events) event.step(model);
    if constexpr ((Advanceable<Agents, M> && ...)) {
        for (auto& event : events) event.advance(model);
    }
    for (auto& event : events) {
        if (event.interval && event.isActive())
            enqueue(event.agent, epochs + event.interval, event.order, event.interval);
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
