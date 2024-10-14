#pragma once

#include <variant>

namespace abmf {

template<typename A, typename M>
concept Stepable = requires(A a, M m) {
    a.step(m);
};

template<typename M, Stepable<M>... Agents>
struct Action {
    size_t time;
    size_t priority;

    void step(M &model) {
        std::visit([&](auto &agent) { agent.get().step(model); }, agents);
    }

private:
    std::variant<Agents...> agents;
};

template<typename M, Stepable<M>... Agents>
class Schedule {};

}
