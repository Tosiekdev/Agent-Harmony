#include <gtest/gtest.h>

#include "../space/Network.hpp"

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
TEST(NetworkTest, AddingNode) {
    abmf::Network<int, int> network;
    network.addNode(5);
    network.addNode(6);
    EXPECT_EQ(network.getNodes().size(), 2);
}

TEST(NetworkTest, AddingEdgeUndirected) {
    abmf::Network<int, int> network;
    auto& n1 = network.addNode(5);
    auto& n2 = network.addNode(6);
    network.addEdge(n1, n2, abmf::EdgeOptions<int>{1, 2});
    for (const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(edge.from, n1);
        EXPECT_EQ(edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    for (const auto& edge : network.getEdges(n2)) {
        EXPECT_EQ(edge.from, n2);
        EXPECT_EQ(edge.to, n1);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
}

TEST(NetworkTest, AddingEdgeUndirectedSecondTime) {
    abmf::Network<int, int> network;
    auto& n1 = network.addNode(5);
    auto& n2 = network.addNode(6);
    auto& n3 = network.addNode(5);
    network.addEdge(n1, n2, abmf::EdgeOptions<int>{1, 2});
    for (const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(edge.from, n1);
        EXPECT_EQ(edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    for (const auto& edge : network.getEdges(n2)) {
        EXPECT_EQ(edge.from, n2);
        EXPECT_EQ(edge.to, n1);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    for (const auto& edge : network.getEdges(n3)) {
        // on purpose, we check if node goes from n1 because n1 and n3 should be identical
        EXPECT_EQ(edge.from, n1);
        EXPECT_EQ(edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
}

TEST(NetworkTest, AddingEdgeDirected) {
    abmf::Network<int, int> network(false);
    auto& n1 = network.addNode(5);
    auto& n2 = network.addNode(6);
    network.addEdge(n1, n2, abmf::EdgeOptions<int>{1, 2});
    for (const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(edge.from, n1);
        EXPECT_EQ(edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
    EXPECT_EQ(network.getEdges(n2).size(), 0);
}

TEST(NetworkTest, DeleteNode) {
    abmf::Network<int, int> network;
    auto& n1 = network.addNode(5);
    auto& n2 = network.addNode(6);
    network.addEdge(n1, n2);
    EXPECT_EQ(network.getNodes().size(), 2);
    EXPECT_EQ(network.getEdges(n2).size(), 1);
    network.deleteNode(n1);
    EXPECT_EQ(network.getNodes().size(), 1);
    EXPECT_EQ(network.getEdges(n2).size(), 0);
}

TEST(NetworkTest, RemoveEdge) {
    abmf::Network<int, int> network;
    auto& n1 = network.addNode(5);
    auto& n2 = network.addNode(6);
    const abmf::Edge<int, int> edge(n1, n2);
    network.addEdge(n1, n2);
    network.removeEdge(edge);
    EXPECT_EQ(network.getEdges(n1).size(), 0);
    EXPECT_EQ(network.getEdges(n2).size(), 0);
}

TEST(NetworkTest, RemoveEdges) {
    abmf::Network<int, int> network;
    auto& n1 = network.addNode(5);
    auto& n2 = network.addNode(6);
    auto& n3 = network.addNode(7);
    network.addEdge(n1, n2);
    network.addEdge(n1, n3);
    network.addEdge(n2, n3);
    network.removeEdges(n1);
    EXPECT_EQ(network.getEdges(n1).size(), 0);
    EXPECT_EQ(network.getEdges(n2).size(), 1);
    EXPECT_EQ(network.getEdges(n3).size(), 1);
}

TEST(NetworkTest, UpdateNode) {
    abmf::Network<X, int> network;
    auto& node = network.addNode(X(5));
    EXPECT_EQ(network.getNodes().size(), 1);
    EXPECT_EQ(node.value, 0);
    X newNode(5);
    newNode.value = 7;
    network.updateNode(newNode);
    EXPECT_EQ(network.getNodes().size(), 1);
    EXPECT_EQ(node.value, 7);
}
}
