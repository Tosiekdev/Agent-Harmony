#pragma once

#include <deque>
#include <tuple>
#include <ranges>

#include "../utilities/Concepts.hpp"

namespace agh {
/**
 * Class template capable of storing agents present in the simulation. It assures that the address of the agent is
 * invariant.
 * @tparam Agents Types of agents that can be present in the model. They must meet ActiveAgent requirements.
 */
template<ActiveAgent... Agents> requires (sizeof...(Agents) > 0)
class Model {
public:
    /**
     * Returns an iteration range over agents of type T; insertion invalidates the range.
     * @tparam T Type of the agents we want to get.
     * @return Range of agent references without structural container operations.
     */
    template<ActiveAgent T>
    std::ranges::subrange<typename std::deque<T>::iterator> getAgents();

    /** @brief Iterate agents of a const model. @tparam T Stored agent type. @return Range of const references. */
    template<ActiveAgent T>
    std::ranges::subrange<typename std::deque<T>::const_iterator> getAgents() const;

    /**
     * Function for adding new agents to the simulation. It differs from emplaceAgent, by adding an agent based on
     * existing object.
     * @tparam T Type of the agent we want to add. It must meet ActiveAgent requirements.
     * @param agent Constant reference to an agent that we want to add to the simulation.
     * @return Reference to the added agent.
     */
    template<ActiveAgent T>
    T& addAgent(const T& agent);

    /**
     * Function for creating agent directly in the simulation state. It differs from addAgent, by creating from given
     * arguments and not by the copy constructor.
     * @tparam T Type of the agent we want to create. It must meet ActiveAgent requirements.
     * @tparam Args Types of the constructor's arguments for type T.
     * @param args Arguments we want to pass to an agent's constructor.
     * @return Reference to the created agent.
     */
    template<ActiveAgent T, typename... Args>
    T& emplaceAgent(Args&&... args);

    /**
     * Function returning quantities of agents, regardless of type.
     * @return Count of the all agents in the model.
     */
    [[nodiscard]] size_t agentCount() const;

    /**
     * Function returning quantity of agents of given type.
     * @tparam T Type of the agents.
     * @return Number of agents of given type.
     */
    template<ActiveAgent T>
    size_t agentCount() const;

protected:
    /** @brief Owned agent storage; derived classes must preserve registered addresses and memberships. */
    std::tuple<std::deque<Agents>...> agents{};
};
}

#include "ModelImpl.hpp"
