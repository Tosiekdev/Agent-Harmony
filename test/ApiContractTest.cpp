#include <gtest/gtest.h>
#include <ranges>
#include <type_traits>
#include "AgentHarmony.hpp"

namespace {
struct A { std::optional<agh::Point> pos; int value{}; bool isActive() const { return true; } };
struct B { std::optional<agh::Point> pos; int value{}; };
struct R { std::optional<agh::RealPoint> pos; int value{}; };
struct OnlyA { void operator()(A&) const {} };
template<typename Space> constexpr bool acceptsPartial = requires(Space& space) { space.apply(OnlyA{}); };
}

template<typename T> constexpr bool exposesClear = requires(T value) { value.clear(); };
static_assert(!acceptsPartial<agh::Field<A, B>>);
static_assert(!acceptsPartial<agh::MultiagentField<A, B>>);
static_assert(!exposesClear<decltype(std::declval<agh::Model<A>&>().getAgents<A>())>);
static_assert(std::is_const_v<std::remove_reference_t<decltype(*std::declval<agh::Network<int, int>::NodeIter>())>>);
static_assert(std::is_const_v<std::remove_reference_t<std::ranges::range_reference_t<decltype(std::declval<const agh::Model<A>&>().getAgents<A>())>>>);

TEST(ApiContractTest, StructuralAccessorsAreReadOnly) {
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::declval<agh::Field<A>&>().getAgent({0, 0}))>>));
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::declval<agh::MultiagentField<A>&>().getAgents({0, 0}))>>));
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::declval<agh::Network<int, int>&>().getNodes())>>));
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(*std::declval<agh::Network<int, int>::NodeIter>())>>));
    EXPECT_FALSE((acceptsPartial<agh::Field<A, B>>));
    EXPECT_FALSE((acceptsPartial<agh::MultiagentField<A, B>>));
}

TEST(ApiContractTest, FieldTraversalRejectsStructuralMutationsAndRecovers) {
    agh::Field<A, B> field(2, 2);
    A a, extra;
    B b;
    field.addAgent(a, {0, 0});
    field.addAgent(b, {1, 0});
    EXPECT_THROW(field.apply([&](auto&) { field.removeAgent(a); }), std::logic_error);
    EXPECT_THROW(field.transform([&](agh::Point, auto&) { field.addAgent(extra, {0, 1}); }), std::logic_error);
    EXPECT_THROW(field.apply([&](auto&) { field.removeAgent(agh::Point{0, 0}); }), std::logic_error);
    field.apply([&](auto&) { field.apply([](auto& agent) { ++agent.value; }); });
    EXPECT_EQ(a.value, 2);
    EXPECT_EQ(b.value, 2);
    EXPECT_TRUE(field.moveAgent(a, {1, 1}));
}

TEST(ApiContractTest, MultiagentAndContinuousTraversalRejectStructuralMutations) {
    agh::MultiagentField<A, B> grid(2, 2);
    A a, extra;
    grid.addAgent(a, {0, 0});
    EXPECT_THROW(grid.apply([&](auto&) { grid.removeAgent(a); }), std::logic_error);
    EXPECT_THROW(grid.transform([&](agh::Point, auto&) { grid.removeAgents({0, 0}); }), std::logic_error);
    EXPECT_THROW(grid.apply([&](auto&) { grid.moveAgent(a, {1, 1}); }), std::logic_error);
    EXPECT_THROW(grid.apply([&](auto&) { grid.addAgent(extra, {1, 1}); }), std::logic_error);
    grid.removeAgent(a);
    EXPECT_FALSE(a.pos);
    agh::ContinuousSpace<R> space(2, 2, 1);
    R r;
    space.addAgent(r, {0, 0});
    EXPECT_THROW(space.apply([&](auto&) { space.moveAgent(r, {1, 1}); }), std::logic_error);
    EXPECT_THROW(space.apply([&](auto&) { space.removeAgent(r); }), std::logic_error);
    space.apply([](R& agent) { ++agent.value; });
    EXPECT_EQ(r.value, 1);
    space.removeAgent(r);
    EXPECT_FALSE(r.pos);
}

TEST(ApiContractTest, ModelRangesRetainMutableDataAndConstIteration) {
    agh::Model<A> model;
    auto& a = model.emplaceAgent<A>();
    for (auto& agent : model.getAgents<A>()) agent.value = 7;
    EXPECT_EQ(a.value, 7);
}
