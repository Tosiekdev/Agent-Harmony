#pragma once

#include <tuple>
#include <vector>

namespace abmf {

template<typename T>
concept Agent = requires(T t) {
    {t.step()};
};

template<Agent... Agents>
class Model {
public:
    template<Agent T>
    std::vector<T>& getAgents();

    template<Agent T>
    void addAgent(const T& agent);

    template<Agent T, typename... Args>
    void emplaceAgent(Args&&... args);

private:
    std::tuple<std::vector<Agents>...> agents;
};

}

#include "ModelImpl.hpp"
