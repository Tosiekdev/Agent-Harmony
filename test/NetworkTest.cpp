#include "AgentHarmony.hpp"
#include <ranges>
#include <type_traits>
#include <gtest/gtest.h>
#include "space/Network.hpp"
#include <gtest/gtest.h>

#include "../include/space/Network.hpp"
#include "../include/utilities/Hash.hpp"

struct X {
    explicit X(const int i) : id(i) {}
    int id;
    int value{};
    bool operator==(const X& x) const {
        return id == x.id;
    }
};

template<>
struct std::hash<X> {
    size_t operator()(const X& x) const noexcept {
        return std::hash<int>()(x.id);
    }
};

namespace test::network {
using Net = agh::Network<int, int>;
using NodeIt = Net::NodeIter;

TEST(NetworkTest, AddingNode) {
    agh::Network<int, int> network;
    network.addNode(5);
    network.addNode(6);
    EXPECT_EQ(network.getNodes().size(), 2);
}

TEST(NetworkTest, GettingEdge) {
    agh::Network<int, int> network;
    network.addEdge(5, 5, agh::EdgeOptions<int>{.label=1});
    agh::Network<int, int>::CEdgePtr edge = network.getEdge(5, 5, 1);
    ASSERT_TRUE(edge);
    EXPECT_EQ(edge->label, 1);
}

TEST(NetworkTest, AddingEdgeUndirected) {
    agh::Network<int, int> network;
    auto& n1 = *network.addNode(5);
    auto& n2 = *network.addNode(6);
    network.addEdge(n1, n2, agh::EdgeOptions<int>{1, 2});
    for (const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(*edge.from, n1);
        EXPECT_EQ(*edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    for (const auto& edge : network.getEdges(n2)) {
        EXPECT_EQ(*edge.from, n2);
        EXPECT_EQ(*edge.to, n1);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
}

TEST(NetworkTest, AddingEdgeUndirectedSecondTime) {
    agh::Network<int, int> network;
    auto& n1 = *network.addNode(5);
    auto& n2 = *network.addNode(6);
    auto& n3 = *network.addNode(5);
    network.addEdge(n1, n2, agh::EdgeOptions<int>{1, 2});
    for (const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(*edge.from, n1);
        EXPECT_EQ(*edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    for (const auto& edge : network.getEdges(n2)) {
        EXPECT_EQ(*edge.from, n2);
        EXPECT_EQ(*edge.to, n1);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    for (const auto& edge : network.getEdges(n3)) {
        // on purpose, we check if node goes from n1 because n1 and n3 should be identical
        EXPECT_EQ(*edge.from, n1);
        EXPECT_EQ(*edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
}

TEST(NetworkTest, AddingEdgeDirected) {
    agh::Network<int, int> network(true);
    auto& n1 = *network.addNode(5);
    auto& n2 = *network.addNode(6);
    network.addEdge(n1, n2, agh::EdgeOptions<int>{1, 2});
    for (const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(*edge.from, n1);
        EXPECT_EQ(*edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    EXPECT_EQ(network.getEdges(n2).size(), 0);
}

TEST(NetworkTest, DeleteNode) {
    agh::Network<int, int> network;
    auto& n1 = *network.addNode(5);
    auto& n2 = *network.addNode(6);
    network.addEdge(n1, n2);
    EXPECT_EQ(network.getNodes().size(), 2);
    EXPECT_EQ(network.getEdges(n2).size(), 1);
    network.deleteNode(n1);
    EXPECT_EQ(network.getNodes().size(), 1);
    EXPECT_EQ(network.getEdges(n2).size(), 0);
}

TEST(NetworkTest, RemoveEdge) {
    agh::Network<int, int> network;
    auto n1 = network.addNode(5);
    auto n2 = network.addNode(6);
    const agh::Edge<int, int> edge(n1, n2);
    network.addEdge(*n1, *n2);
    network.removeEdge(edge);
    EXPECT_EQ(network.getEdges(*n1).size(), 0);
    EXPECT_EQ(network.getEdges(*n2).size(), 0);
}

TEST(NetworkTest, RemoveEdgeFromTo) {
    agh::Network<int, int> network;
    auto n1 = network.addNode(5);
    auto n2 = network.addNode(6);
    const agh::Edge<int, int> edge(n1, n2);
    network.addEdge(*n1, *n2);
    network.removeEdge(*n1, *n2);
    EXPECT_EQ(network.getEdges(*n1).size(), 0);
    EXPECT_EQ(network.getEdges(*n2).size(), 0);
}

TEST(NetworkTest, RemoveEdges) {
    agh::Network<int, int> network;
    auto& n1 = *network.addNode(5);
    auto& n2 = *network.addNode(6);
    auto& n3 = *network.addNode(7);
    network.addEdge(n1, n2);
    network.addEdge(n1, n3);
    network.addEdge(n2, n3);
    network.removeEdges(n1);
    EXPECT_EQ(network.getEdges(n1).size(), 0);
    EXPECT_EQ(network.getEdges(n2).size(), 1);
    EXPECT_EQ(network.getEdges(n3).size(), 1);
}

TEST(NetworkTest, UpdateNode) {
    agh::Network<X, int> network;
    auto& node = *network.addNode(X(5));
    EXPECT_EQ(network.getNodes().size(), 1);
    EXPECT_EQ(node.value, 0);
    X newNode(5);
    newNode.value = 7;
    network.updateNode(newNode);
    EXPECT_EQ(network.getNodes().size(), 1);
    EXPECT_EQ(node.value, 7);
}

TEST(NetworkTest, GetNeighbourhood) {
    Net network;
    network.addEdge(5, 6);
    network.addEdge(5, 7);
    network.addEdge(5, 8);
    EXPECT_EQ(network.getNodes().size(), 4);
    const auto nghs = network.getNeighborhood(5);
    EXPECT_EQ(nghs.size(), 3);
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 8; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 7; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 6; }) != nghs.end());
}

TEST(NetworkTest, GetNeighbourhoodRadiusGreaterThanOne) {
    Net network;
    network.addEdge(1, 2);
    network.addEdge(1, 3);
    network.addEdge(2, 4);
    network.addEdge(5, 4);
    EXPECT_EQ(network.getNodes().size(), 5);
    auto nghs = network.getNeighborhood(1, 2);
    EXPECT_EQ(nghs.size(), 3);
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 2; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 3; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 4; }) != nghs.end());
    nghs = network.getNeighborhood(1, 3);
    EXPECT_EQ(nghs.size(), 4);
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 2; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 3; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 4; }) != nghs.end());
    EXPECT_TRUE(std::ranges::find_if(nghs, [](const NodeIt n) { return *n == 5; }) != nghs.end());
}
}

namespace test::network_safety {

TEST(NetworkSafetyTest, DeleteReinsertAndRemoveIncidentEdges) {
    for (bool directed : {false, true}) {
        agh::Network<int, int> graph(directed);
        graph.addEdge(1, 2, agh::EdgeOptions<int>{1, 2.0});
        graph.addEdge(1, 2, agh::EdgeOptions<int>{2, 3.0});
        graph.addEdge(2, 3);
        graph.addEdge(2, 2);
        const int& ownedNode = *graph.addNode(2);
        graph.deleteNode(ownedNode);
        EXPECT_FALSE(graph.hasNode(2));
        EXPECT_TRUE(graph.getEdges(1).empty());
        EXPECT_TRUE(graph.getEdges(3).empty());
        graph.deleteNode(2);
        auto node = graph.addNode(2);
        ASSERT_EQ(graph.getNodes().size(), 3);
        EXPECT_EQ(*node, 2);
        EXPECT_TRUE(graph.getEdges(2).empty());
        graph.addEdge(2, 1);
        EXPECT_EQ(graph.getEdges(2).size(), 1);
        graph.deleteNode(2);
        EXPECT_TRUE(graph.getEdges(1).empty());
    }
}

TEST(NetworkSafetyTest, MissingOperationsDoNotInsertNodes) {
    agh::Network<int, int> graph;
    graph.addNode(1);
    EXPECT_THROW(graph.getEdges(99), std::out_of_range);
    graph.removeEdge(1, 99);
    graph.removeEdge(98, 99);
    graph.removeEdges(99);
    EXPECT_EQ(graph.getNodes().size(), 1);
    EXPECT_EQ(graph.getEdge(1, 99, 1), nullptr);
    EXPECT_TRUE(graph.getEdges(1).empty());
}

TEST(NetworkSafetyTest, DirectedBulkRemovalPreservesIncomingEdges) {
    agh::Network<int, int> graph(true);
    graph.addEdge(1, 2);
    graph.addEdge(2, 3);
    graph.addEdge(2, 2);
    graph.removeEdges(2);
    EXPECT_EQ(graph.getEdges(1).size(), 1);
    EXPECT_TRUE(graph.getEdges(2).empty());
    graph.deleteNode(2);
    EXPECT_TRUE(graph.getEdges(1).empty());
}

TEST(NetworkSafetyTest, RemoveGraphOwnedEdgeAndSelfLoop) {
    for (bool directed : {false, true}) {
        agh::Network<int, int> graph(directed);
        graph.addEdge(1, 2, agh::EdgeOptions<int>{7, 2.0});
        graph.addEdge(1, 2, agh::EdgeOptions<int>{8, 3.0});
        graph.removeEdge(*graph.getEdge(1, 2, 7));
        EXPECT_EQ(graph.getEdge(1, 2, 7), nullptr);
        EXPECT_NE(graph.getEdge(1, 2, 8), nullptr);
        graph.addEdge(1, 1, agh::EdgeOptions<int>{9, 1.0});
        graph.removeEdge(*graph.getEdge(1, 1, 9));
        EXPECT_EQ(graph.getEdge(1, 1, 9), nullptr);
    }
}

static_assert(!std::is_copy_constructible_v<agh::Network<int, int>>);
static_assert(!std::is_copy_assignable_v<agh::Network<int, int>>);

TEST(NetworkSafetyTest, CopyingIsDisabled) {
    EXPECT_FALSE((std::is_copy_constructible_v<agh::Network<int, int>>));
    EXPECT_FALSE((std::is_copy_assignable_v<agh::Network<int, int>>));
}

TEST(NetworkSafetyTest, MovesKeepAllRelationshipsInDestination) {
    agh::Network<int, int> destination;
    destination.addEdge(8, 9);
    {
        agh::Network<int, int> source(true);
        source.addEdge(1, 2, agh::EdgeOptions<int>{7, 2.0});
        source.addEdge(2, 2);
        const int* address = &*source.addNode(1);
        agh::Network<int, int> moved(std::move(source));
        EXPECT_EQ(&*moved.addNode(1), address);
        destination = std::move(moved);
        EXPECT_EQ(&*destination.addNode(1), address);
    }
    EXPECT_FALSE(destination.hasNode(8));
    EXPECT_NE(destination.getEdge(1, 2, 7), nullptr);
    destination.removeEdges(2);
    EXPECT_EQ(destination.getEdges(1).size(), 1);
    destination.deleteNode(2);
    EXPECT_TRUE(destination.getEdges(1).empty());
    destination.addNode(2);
    EXPECT_TRUE(destination.getEdges(2).empty());
}

}

namespace test::api_network {
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
static_assert(std::is_const_v<std::remove_reference_t<decltype(*std::declval<agh::Network<int, int>::NodeIter>())>>);
TEST(ApiContractTest, StructuralAccessorsAreReadOnlyNetwork) {
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(std::declval<agh::Network<int, int>&>().getNodes())>>));
    EXPECT_TRUE((std::is_const_v<std::remove_reference_t<decltype(*std::declval<agh::Network<int, int>::NodeIter>())>>));
}
}
