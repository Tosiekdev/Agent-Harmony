#include "AgentHarmony.hpp"
#include <ranges>
#include <type_traits>
#include <gtest/gtest.h>

#include <optional>

#include "../include/model/Model.hpp"

namespace test::model {
struct MyAgent {
    explicit MyAgent(const int pId) : id(pId) {}
    int id;
    bool isActive() {
        return true;
    }
};

struct NotMyAgent {
    explicit NotMyAgent(const int pId) : id(pId) {}
    int id;
    bool isActive() {
        return true;
    }
};

struct MyModel : agh::Model<MyAgent> {};

struct MyModel2 : agh::Model<MyAgent, NotMyAgent> {};

TEST(ModelTest, AddingAgent) {
    MyModel2 m;
    MyAgent a1{1};
    NotMyAgent a2{2};
    m.addAgent(a1);
    m.addAgent(a2);
    ASSERT_EQ(m.agentCount(), 2);
    ASSERT_EQ(m.agentCount<MyAgent>(), 1);
    ASSERT_EQ(m.agentCount<NotMyAgent>(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
    EXPECT_EQ(m.getAgents<NotMyAgent>()[0].id, 2);
}

TEST(ModelTest, EmplacingAgent) {
    MyModel m;
    m.emplaceAgent<MyAgent>(1);
    ASSERT_EQ(m.agentCount(), 1);
    ASSERT_EQ(m.agentCount<MyAgent>(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
}

TEST(ModelTest, AddingAgentDifferentTypes) {
    MyModel2 m;
    m.emplaceAgent<MyAgent>(1);
    m.emplaceAgent<NotMyAgent>(2);
    ASSERT_EQ(m.agentCount(), 2);
    ASSERT_EQ(m.agentCount<MyAgent>(), 1);
    ASSERT_EQ(m.agentCount<NotMyAgent>(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
    EXPECT_EQ(m.getAgents<NotMyAgent>()[0].id, 2);
}
}

namespace {
struct CountedAgent {
    static inline int copies = 0;
    CountedAgent() = default;
    CountedAgent(const CountedAgent&) { ++copies; }
    bool isActive() const { return true; }
};
struct NoncopyableAgent {
    NoncopyableAgent() = default;
    NoncopyableAgent(const NoncopyableAgent&) = delete;
    bool isActive() const { return true; }
};
}

TEST(ModelCountTest, CountingDoesNotCopyAgents) {
    agh::Model<CountedAgent> model;
    EXPECT_EQ(std::as_const(model).agentCount(), 0);
    for (int i = 0; i < 3; ++i) model.emplaceAgent<CountedAgent>();
    CountedAgent::copies = 0;
    EXPECT_EQ(std::as_const(model).agentCount(), 3);
    EXPECT_EQ(CountedAgent::copies, 0);
}

TEST(ModelCountTest, NoncopyableMixedAgentsCanBeCounted) {
    agh::Model<CountedAgent, NoncopyableAgent> model;
    model.emplaceAgent<NoncopyableAgent>();
    model.emplaceAgent<CountedAgent>();
    EXPECT_EQ(std::as_const(model).agentCount(), 2);
}

namespace test::api_model {
namespace {
struct A { std::optional<agh::Point> pos; int value{}; bool isActive() const { return true; } };
struct B { std::optional<agh::Point> pos; int value{}; };
struct R { std::optional<agh::RealPoint> pos; int value{}; };
struct S { std::optional<agh::RealPoint> pos; int value{}; };
struct OnlyA { void operator()(A&) const {} };
struct OnlyR { void operator()(R&) const {} };
struct EveryAgent { void operator()(auto&) const {} };
struct OnlyPointA { void operator()(agh::Point, A&) const {} };
struct EveryPointAgent { void operator()(agh::Point, auto&) const {} };
template<typename Space> constexpr bool acceptsPartial = requires(Space& space) { space.apply(OnlyA{}); };
template<typename Space, typename Visitor> constexpr bool acceptsVisitor =
    requires(Space& space, Visitor visitor) { space.apply(visitor); };
template<typename Space, typename Visitor> constexpr bool acceptsTransform =
    requires(Space& space, Visitor visitor) { space.transform(visitor); };
template<typename Space, typename Agent> constexpr bool acceptsAnyMembership =
    requires(Space& space, Agent& agent) { space.addAgent(agent, {}); } ||
    requires(Space& space, Agent& agent) { space.moveAgent(agent, {}); } ||
    requires(Space& space, Agent& agent) { space.removeAgent(agent); };
}


template<typename T> constexpr bool exposesClear = requires(T value) { value.clear(); };
static_assert(!exposesClear<decltype(std::declval<agh::Model<A>&>().getAgents<A>())>);
static_assert(std::is_const_v<std::remove_reference_t<std::ranges::range_reference_t<decltype(std::declval<const agh::Model<A>&>().getAgents<A>())>>>);
TEST(ApiContractTest, ModelRangesRetainMutableDataAndConstIteration) {
    agh::Model<A> model;
    auto& a = model.emplaceAgent<A>();
    for (auto& agent : model.getAgents<A>()) agent.value = 7;
    EXPECT_EQ(a.value, 7);
}
}
