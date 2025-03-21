#pragma once

#include "Edge.hpp"

#include <list>
#include <unordered_map>
#include <unordered_set>

namespace abmf {
/**
 * Class template which aims to represent graphs. Both directed and undirected. It imposes only small requirements for
 * the type of nodes and type of edges' labels. If there is more than one connection between two nodes, then they have
 * to have different label.
 * @tparam N Type of the nodes of the graph. It should meet Node requirements.
 * @tparam L Type of the labels of graph's edges. It should meet Label requirements.
 */
template<Node N, Label L>
class Network {
public:
    using EdgeT = Edge<N, L>;
    using CEdgePtr = const EdgeT*;
    using EdgeSet = std::unordered_set<EdgeT>;
    explicit Network(const bool isDirected = true) : directed(isDirected) {};

    /**
     * Adds given node to the network, without connecting it to any other node. If node is already present in the
     * network, reference to the existing object will be returned.
     * @param node Node to add.
     * @return Reference to the added node.
     */
    N& addNode(const N& node);

    /**
     * Removes given node from the network. Added node won't have any attributes.
     * @param node Node to remove.
     */
    void deleteNode(N& node);

    /**
     * Returns reference to the std::list containing all the network nodes. Real nodes, not pointer to them, or
     * something like that. However, user should not modify contents of this list.
     * @return Reference to the list with all nodes.
     */
    std::list<N>& getNodes() { return nodes; }

    /**
     * Returns reference to the const std::list containing all the network nodes. Real nodes, not pointer to them, or
     * something like that.
     * @return Reference to the constant list with all nodes.
     */
    const std::list<N>& getNodes() const { return nodes; }

    /**
     * Returns all the edges that have a source in the given node. For undirected netowrks it means that all the edges
     * connected to that node are returned.
     * @param node In this node all the returned nodes have a beginning.
     * @return Reference to a constant set with all the edges starting in the given node.
     */
    const EdgeSet& getEdges(const N& node) const;

    /**
     *
     * @param from Source node of the edge.
     * @param to End node of the edge.
     * @param label Label of the edge.
     * @return Optional pointer to the const found edge.
     */
    CEdgePtr getEdge(const N& from, const N& to, L label);

    /**
     * Adds connection between two nodes. Added node won't have any additional attributes. If the network is undirected,
     * edge from the second node to the first will also be added. If any of the given nodes does not exist in the
     * network, it will be added.
     * @param from Node where edge starts.
     * @param to Node where edge ends.
     */
    void addEdge(const N& from, const N& to);

    /**
     * Adds connection between two nodes. Added node will have attributes specified in sent EdgeOption object. If the
     * network is undirected, edge from the second node to first, with specified attributes, will also be added. If any
     * of the given nodes does not exist in the network, it will be added.
     * @param from Node where edge starts.
     * @param to Node where edge ends.
     * @param options Properties of the edge: weight or label.
     */
    void addEdge(const N& from, const N& to, EdgeOptions<L> options);

    /**
     * Finds node equivalent to passed in the network and updates it value accordingly. This function makes sense only
     * if not all attributes of the two nodes must be equal to consider those nodes equivalent.
     * @param node New value of the node.
     */
    void updateNode(const N& node);

    /**
     * Removes unlabeled connection between two nodes. If the network is undirected it also removes symmetrical edge
     * from the second node to the first.
     * @param from Node where the edge starts.
     * @param to Node where the edge ends.
     */
    void removeEdge(const N& from, const N& to);

    /**
     * Removes specified edge from the network. If the network is undirected it also removes symmetrical edge from the
     * second node to the first.
     * @param edge Edge to remove.
     */
    void removeEdge(const EdgeT& edge);

    /**
     * Removes all edges starting at the given node. If the network is undirected it also removes edges ending in that
     * node.
     * @param node Source edge of the edges to remove.
     */
    void removeEdges(const N& node);

    /**
     * Checks if given node is present in the network.
     * @param node Node to check if is in the network.
     * @return True if node equivalent to passed is present in the network, false otherwise.
     */
    bool hasNode(const N& node);

    /**
     * Gets all of the nodes which can be found in the distance not greater than specified. Optionally it also return
     * the central node.
     * @param node Node which neighbors we want to obtain.
     * @param radius Distance in which we want to search for the neighbors.
     * @param center Boolean value indicating if function should also return the central node.
     * @return Vector containing pointers to the nodes which are neighbors of the central node.
     */
    std::vector<N*> getNeighborhood(const N& node, size_t radius = 1, bool center = false);

private:
    std::list<N> nodes;
    std::unordered_map<N, N*> lookup;
    std::unordered_map<const N*, EdgeSet> edges;
    bool directed;
};
}

#include "NetworkImpl.hpp"
