#pragma once

#include <compare>
#include <functional>
#include <variant>
#include <queue>

#include "AgentBase.hpp"

namespace abmf {
template<typename A, typename M>
concept Schedulable = requires(A a, M m) {
    a.step(m);
} && ActiveAgent<A>;

template<typename M>
concept SimState = requires(M m) {
    m.beforeStep();
    m.afterStep();
    { m.shouldEnd(int{}) } -> std::same_as<bool>;
};

template<typename M, Schedulable<M>... Agents>
struct Action {
    Action(auto& pAgent, const size_t pTime, const size_t pPriority, const size_t pInterval = 0)
        : time(pTime), priority(pPriority), interval(pInterval), agent(&pAgent) {}

    Action(const std::variant<Agents*...> pAgent, const size_t pTime, const size_t pPriority,
           const size_t pInterval = 0) : time(pTime), priority(pPriority), interval(pInterval), agent(pAgent) {}

    void step(M& model) {
        std::visit([&](auto agent) { agent->step(model); }, agent);
    }

    [[nodiscard]] bool isActive() const {
        return std::visit([](auto agent) { return agent->isActive(); }, agent);
    }

    size_t time;
    size_t priority;
    size_t interval;
    std::variant<Agents*...> agent;

    auto operator<=>(const Action& rhs) const {
        if (auto relation = time <=> rhs.time; relation != 0) {
            return relation;
        }
        return priority <=> rhs.priority;
    }
};

template<SimState M, Schedulable<M>... Agents>
class Schedule {
public:
    using ActionItem = Action<M, Agents...>;
    explicit Schedule(M& pModel) : model(pModel), epochs(0) {}
    void scheduleOnce(auto& agent, size_t time, size_t priority);
    void scheduleRepeating(auto& agent, size_t time, size_t priority, size_t interval);
    void scheduleRepeating(auto& agent, size_t time, size_t priority);
    void step();
    void execute();

    [[nodiscard]] size_t getEpochs() const;

private:
    M& model;
    size_t epochs;
    std::priority_queue<ActionItem, std::vector<ActionItem>, std::greater<ActionItem>> actions;
};
}

#include "ScheduleImpl.hpp"
