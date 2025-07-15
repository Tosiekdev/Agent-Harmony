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
