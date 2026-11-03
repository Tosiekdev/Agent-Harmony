#include <space/Network.hpp>

int main() {
    agh::Network<int, int> graph(true);
    graph.addEdge(1, 2, agh::EdgeOptions<int>{7, 2.0});
    graph.addEdge(2, 3);
    graph.removeEdges(2); // Outgoing only: 1 -> 2 remains.
    if (!graph.getEdge(1, 2, 7) || !graph.getEdges(2).empty()) return 1;
    graph.deleteNode(2);
    graph.addNode(2); // Safe reinsertion after deletion.
    return graph.getEdges(1).empty() && graph.getEdges(2).empty() ? 0 : 1;
}
