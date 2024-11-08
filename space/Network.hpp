#pragma once

#include "Edge.hpp"

#include <list>
#include <unordered_map>
#include <unordered_set>

namespace abmf {
template<Node N, Label L>
class Network {
public:
    using EdgeT = Edge<N, L>;
    using EdgeSet = std::unordered_set<EdgeT>;
    explicit Network(const bool isDirected = false) : directed(isDirected) {};

    N& addNode(const N& node);
    void deleteNode(const N& node);

    const std::list<N>& getNodes() const { return nodes; }
    const EdgeSet& getEdges(const N& node) const { return edges.at(&node); }

    bool hasNode(const N& node);

    void addEdge(N& from, N& to, EdgeOptions<L> options);

    std::unordered_set<N*> getNeighborhood(const N& node, size_t radius, bool center);

private:
    std::list<N> nodes;
    std::unordered_map<const N*, EdgeSet> edges;
    bool directed;
};
}

#include "NetworkImpl.hpp"
