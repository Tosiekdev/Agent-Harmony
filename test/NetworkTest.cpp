#include <gtest/gtest.h>

#include "../space/Network.hpp"

namespace test::network {
TEST(NetworkTest, AddingNode) {
    abmf::Network<int, int> network;
    network.addNode(5);
    network.addNode(6);
    EXPECT_EQ(network.getNodes().size(), 2);
}

TEST(NetworkTest, AddingEdge) {
    abmf::Network<int, int> network;
    auto n1 = network.addNode(5);
    auto n2 = network.addNode(6);
    network.addEdge(n1, n2, abmf::EdgeOptions<int>{1, 2});
    for(const auto& edge : network.getEdges(n1)) {
        EXPECT_EQ(edge.from, n1);
        EXPECT_EQ(edge.to, n2);
        EXPECT_EQ(edge.label, 1);
        EXPECT_EQ(edge.weight, 2);
    }
}
}
