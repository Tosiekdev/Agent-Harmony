#pragma once

#include "Edge.hpp"

#include <list>
#include <unordered_map>
#include <unordered_set>

namespace abmf {
template<Node N, Label L>
class Network {
public:
    explicit Network(const bool isDirected = false) : directed(isDirected) {};

    N& addNode(const N& node);
    void deleteNode(const N& node);

    void addEdge(N& from, N& to, EdgeOptions<L> options);

    std::unordered_set<N*> getNeighborhood(const N& node, size_t radius, bool center);

private:
    std::list<N> nodes;
    std::unordered_map<N*, std::unordered_set<Edge<N, L>>> edges;
    bool directed;
};
}

#include "NetworkImpl.hpp"
