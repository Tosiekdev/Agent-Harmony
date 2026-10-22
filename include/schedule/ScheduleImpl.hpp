#pragma once

#include <limits>
#include <stdexcept>

namespace agh {
template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleOnce(auto& agent, const size_t time, const size_t order) {
    if (time < epochs) throw std::invalid_argument("Cannot schedule an event in the past");
    if (nextSequence == std::numeric_limits<size_t>::max()) throw std::overflow_error("Event sequence exhausted");
    actions.emplace(agent, time, order, 0, nextSequence);
    ++nextSequence;
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::scheduleRepeating(auto& agent, const size_t time, const size_t order,
                                               const size_t interval) {
    if (time < epochs || interval == 0)
        throw std::invalid_argument("Repeating events require a current/future time and positive interval");
    if (nextSequence == std::numeric_limits<size_t>::max()) throw std::overflow_error("Event sequence exhausted");
    actions.emplace(agent, time, order, interval, nextSequence);
    ++nextSequence;
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::step() {
    if (actions.empty()) {
        if (epochs == std::numeric_limits<size_t>::max()) throw std::overflow_error("Schedule time exhausted");
        ++epochs;
        model.beforeStep();
        model.afterStep();
        return;
    }
    const size_t batchTime = actions.top().time;
    std::vector<ActionItem> events;
    events.reserve(actions.size());
    while (!actions.empty() && actions.top().time == batchTime) {
        if (actions.top().isActive() && actions.top().interval > std::numeric_limits<size_t>::max() - batchTime) {
            for (const auto& event : events) actions.push(event);
            throw std::overflow_error("Repeating event time exhausted");
        }
        events.push_back(actions.top());
        actions.pop();
    }
    std::vector<bool> executed(events.size(), false);
    epochs = batchTime;
    model.beforeStep();
    for (size_t i = 0; i < events.size(); ++i) {
        if (events[i].isActive()) {
            events[i].step(model);
            executed[i] = true;
        }
    }
    for (size_t i = 0; i < events.size(); ++i) {
        if (executed[i]) events[i].advance(model);
    }
    for (size_t i = 0; i < events.size(); ++i) {
        const auto& event = events[i];
        if (executed[i] && event.interval && event.isActive())
            scheduleRepeating(event.agent, epochs + event.interval, event.order, event.interval);
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
