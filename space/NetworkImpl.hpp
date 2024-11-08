#pragma once

#include "Network.hpp"

#include <queue>

namespace abmf {
template<Node N, Label L>
N& Network<N, L>::addNode(const N& node) {
    nodes.push_back(node);
    return nodes.back();
}

template<Node N, Label L>
void Network<N, L>::deleteNode(const N& node) {
    if (!directed) {
        std::unordered_set<N*> nghs;
        for (const auto& e : edges[&node]) {
            nghs.insert(&e.to);
        }
        for (auto n : nghs) {
            std::erase_if(edges[n], [&](const auto& e){ return e.to == node;});
        }
    } else {
        for (auto& [n, e] : edges) {
            std::erase_if(e[n], [&](const auto& edge){ return edge.to == node;});
        }
    }
    edges.erase(&node);
    nodes.erase(&node);
}

template<Node N, Label L>
bool Network<N, L>::hasNode(const N& node) {
    return std::find(nodes.begin(), nodes.end(), node) == nodes.end();
}

template<Node N, Label L>
void Network<N, L>::addEdge(N& from, N& to, EdgeOptions<L> options) {
    if (!hasNode(from)) {
        from = addNode(from);
    }
    if (!hasNode(to)) {
        to = addNode(to);
    }
    edges[&from].insert(Edge<N, L>(from, to, options));
    if (directed) {
        edges[&to].insert(Edge<N, L>(to, from, options));
    }
}

template<Node N, Label L>
std::unordered_set<N*> Network<N, L>::getNeighborhood(const N& node, const size_t radius,
                                                                const bool center) {
    std::unordered_set<N*> neighborhood;

    if (!radius) {
        return neighborhood;
    }

    std::unordered_set<N*> toVisit;
    for (const auto& edge : edges[&node]) {
        if (neighborhood.insert(&edge.to).second) {
            toVisit.insert(&edge.to);
        }
    }

    for (size_t i = 2; i < radius; ++i) {
        if (toVisit.empty()) {
            break;
        }

        std::unordered_set<N*> nextToVisit;
        for (const auto& n : toVisit) {
            for (const auto& edge : edges[&n]) {
                if (neighborhood.insert(&edge.to).second) {
                    nextToVisit.insert(&edge.to);
                }
            }
        }
        toVisit = std::move(nextToVisit);
    }

    if (center) {
        neighborhood.insert(&node);
    }

    return neighborhood;
}
}
