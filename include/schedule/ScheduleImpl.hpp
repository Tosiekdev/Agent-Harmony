#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <span>
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
void Schedule<M, Agents...>::considerBacklog() {
    if (nextBacklog != backlog.size()) return;
    if (actions.size() < minSampleQueueSize) {
        nextSampleQueueSize = minSampleQueueSize;
        return;
    }
    if (actions.size() < nextSampleQueueSize) return;

    std::array<size_t, 1024> times;
    auto& heap = actions.storage();
    const auto stride = heap.size() / times.size();
    for (size_t i = 0; i < times.size(); ++i) times[i] = heap[i * stride].time;
    std::sort(times.begin(), times.end());
    const auto distinct = static_cast<size_t>(std::unique(times.begin(), times.end()) - times.begin());
    // Sort only when at least a quarter of sampled times repeat an earlier sample.
    if (distinct > times.size() * 3 / 4) {
        nextSampleQueueSize = heap.size() * 2;
        return;
    }

    std::sort(heap.begin(), heap.end());
    backlog.clear();
    backlog.swap(heap);
    nextBacklog = 0;
    nextSampleQueueSize = minSampleQueueSize;
}

template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
void Schedule<M, Agents...>::step() {
    considerBacklog();
    const bool hasBacklog = nextBacklog != backlog.size();
    if (actions.empty() && !hasBacklog) {
        ++epochs;
        model.beforeStep();
        model.afterStep();
        return;
    }

    std::span<ActionItem> events;
    auto backlogEnd = nextBacklog;
    if (hasBacklog && (actions.empty() || backlog[nextBacklog].time <= actions.top().time)) {
        epochs = backlog[nextBacklog].time;
        do { ++backlogEnd; }
        while (backlogEnd != backlog.size() && backlog[backlogEnd].time == epochs);
        events = std::span(backlog).subspan(nextBacklog, backlogEnd - nextBacklog);
    } else {
        epochs = actions.top().time;
    }

    std::vector<ActionItem> selected;
    if (!actions.empty() && actions.top().time == epochs) {
        selected.reserve(actions.size() + events.size());
        auto pending = events.begin();
        do {
            while (pending != events.end() && *pending < actions.top()) {
                selected.push_back(*pending++);
            }
            selected.push_back(actions.top());
            actions.pop();
        } while (!actions.empty() && actions.top().time == epochs);
        while (pending != events.end()) selected.push_back(*pending++);
        events = selected;
    }
    // Finish selecting before callbacks can enqueue any more events for this tick.
    nextBacklog = backlogEnd;
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
