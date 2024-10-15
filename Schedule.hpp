#pragma once

#include <compare>
#include <functional>
#include <variant>
#include <queue>

namespace abmf {
template<typename A, typename M>
concept Stepable = requires(A a, M m) {
    a.step(m);
};

template<typename M, Stepable<M>... Agents>
struct Action {
    Action(auto& pAgent, const size_t pTime, const size_t pPriority, const size_t pInterval = 0)
        : time(pTime), priority(pPriority), interval(pInterval), agent(pAgent) {}

    void step(M& model) {
        std::visit([&](auto& agent) { agent.get().step(model); }, agent);
    }

    size_t time;
    size_t priority;
    size_t interval;
    std::variant<std::reference_wrapper<Agents>...> agent;

    auto operator<=>(const Action& rhs) const {
        if (auto relation = time <=> rhs.time; relation != 0) {
            return relation;
        }
        return priority <=> rhs.priority;
    }
};

template<typename M, Stepable<M>... Agents>
class Schedule {
public:
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
    std::priority_queue<Action<M, Agents>...> actions;
};
}

#include "ScheduleImpl.hpp"
