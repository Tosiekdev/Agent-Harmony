#pragma once

#include <functional>
#include <variant>

namespace abmf {
template<typename A, typename M>
concept Stepable = requires(A a, M m) {
    a.step(m);
};

template<typename M, Stepable<M>... Agents>
struct Action {
    Action(auto& pAgent, size_t const pTime, size_t const pPriority) : time(pTime), priority(pPriority), agent(pAgent) {}

    void step(M& model) {
        std::visit([&](auto& agent) { agent.get().step(model); }, agent);
    }

    size_t time;
    size_t priority;

private:
    std::variant<std::reference_wrapper<Agents>...> agent;
};

template<typename M, Stepable<M>... Agents>
class Schedule {};
}
