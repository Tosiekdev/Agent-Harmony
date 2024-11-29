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
    explicit Network(const bool isDirected = true) : directed(isDirected) {};

    N& addNode(const N& node);
    void deleteNode(N& node);

    std::list<N>& getNodes() { return nodes; }
    const std::list<N>& getNodes() const { return nodes; }
    const EdgeSet& getEdges(const N& node) const;

    void addEdge(const N& from, const N& to);
    void addEdge(const N& from, const N& to, EdgeOptions<L> options);

    void updateNode(const N& node);

    void removeEdge(const EdgeT& edge);
    void removeEdges(const N& node);

    bool hasNode(const N& node);

    std::vector<N*> getNeighborhood(const N& node, size_t radius=1, bool center=false);

private:
    std::list<N> nodes;
    std::unordered_map<N, N*> lookup;
    std::unordered_map<const N*, EdgeSet> edges;
    bool directed;
};
}

#include "NetworkImpl.hpp"
