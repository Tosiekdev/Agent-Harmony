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
