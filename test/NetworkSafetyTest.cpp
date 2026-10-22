#include <gtest/gtest.h>
#include "space/Network.hpp"

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
