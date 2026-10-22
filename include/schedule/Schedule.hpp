#pragma once

#include <variant>
#include <queue>
#include <compare>
#include <cstddef>
#include <functional>
#include <vector>

#include "../utilities/Concepts.hpp"

namespace agh {
namespace action {
template<typename M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
struct Action {
    Action(auto& pAgent, const size_t pTime, const size_t pPriority, const size_t pInterval = 0, const size_t pSequence = 0)
        : time(pTime), order(pPriority), interval(pInterval), agent(&pAgent), sequence(pSequence) {}

    Action(const std::variant<Agents*...> pAgent, const size_t pTime, const size_t pPriority,
           const size_t pInterval = 0, const size_t pSequence = 0) : time(pTime), order(pPriority), interval(pInterval), agent(pAgent), sequence(pSequence) {}

    void step(M& model) {
        std::visit([&](auto agent) { agent->step(model); }, agent);
    }

    void advance(M& model) {
        std::visit([&](auto pointer) {
            if constexpr (Advanceable<std::remove_pointer_t<decltype(pointer)>, M>) pointer->advance(model);
        }, agent);
    }

    [[nodiscard]] bool isActive() const {
        return std::visit([](auto agent) { return agent->isActive(); }, agent);
    }

    size_t time;
    size_t order;
    size_t interval;
    std::variant<Agents*...> agent;

    size_t sequence = 0;

    auto operator<=>(const Action& rhs) const {
        if (auto relation = time <=> rhs.time; relation != 0) {
            return relation;
        }
        if (auto relation = order <=> rhs.order; relation != 0) return relation;
        return sequence <=> rhs.sequence;
    }
};
}

/**
 * Class template representing schedule for the simulation, where state is represented by object of class M, and
 * simulation's agents types are in Agents. It is responsible for running simulation and scheduling events.
 * @tparam M Type of the object representing simulation state. It should meet requirements of SimState.
 * @tparam Agents Types of the agents present in the simulation. They should meet requirements of Schedulable<M>
 */
template<SimState M, Schedulable<M>... Agents> requires (sizeof...(Agents) > 0)
class Schedule {
public:
    using ActionItem = action::Action<M, Agents...>;
    /**
     * Constructs Schedule correlated with given model.
     * @param pModel Reference to an object representing simulation state.
     */
    explicit Schedule(M& pModel) : model(pModel), epochs(0) {}

    /**
     * Schedules agent's action only once. It will be executed in specified time. If two agents are scheduled for the
     * same step, one with the lower order will be executed first.
     * @param agent Agent whose action will be executed.
     * @param time Time step in which action will be executed.
     * @param order Order value. The lower, the better.
     */
    void scheduleOnce(auto& agent, size_t time, size_t order);

    /**
     * Schedules agent's action which will be repeated every specified number of steps, beginning from the specified
     * time. If two agents are scheduled for the same step, one with the lower order will be executed first.
     * @param agent Agent to schedule.
     * @param time Time step at which agent's action should be invoked.
     * @param order Order value. The lower, the better.
     * @param interval Number of iterations after which action should be invoked again. Default value is one.
     */
    void scheduleRepeating(auto& agent, size_t time, size_t order, size_t interval = 1);

    /**
     * Executes one step of the simulation. It consists of calling beforeStep method of the model, executing step
     * method for every agent scheduled for that time step, potentially calling advance method for mentioned agents and
     * finally, calling afterStep on the model.
     */
    void step();

    /**
     * Calls step method, until model's shouldEnd method returns true.
     */
    void execute();

    /**
     * Returns number of steps that were executed. It doesn't necessarily mean, that this is a number of times the step
     * function was called.
     * @return Number of steps that were executed from the beginning of the simulation.
     */
    [[nodiscard]] size_t getEpochs() const;

    /**
     *
     * @return Logical value indicating if simulation should continue.
     */
    [[nodiscard]] bool isActive() const;

private:
    M& model;
    size_t epochs;
    size_t nextSequence = 0;
    std::priority_queue<ActionItem, std::vector<ActionItem>, std::greater<ActionItem>> actions;
};
}

#include "ScheduleImpl.hpp"
