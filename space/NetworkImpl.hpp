#pragma once

#include "Network.hpp"

#include <queue>

namespace abmf {
template<typename Node, typename Label> requires requires { { std::hash<Label>() } -> std::same_as<unsigned long>; }
Node& Network<Node, Label>::addNode(const Node& node) {
    return *nodes.emplace_back(node);
}

template<typename Node, typename Label> requires requires { { std::hash<Label>() } -> std::same_as<unsigned long>; }
void Network<Node, Label>::addEdge(Node& from, Node& to, EdgeOptions<Label> options) {
    if (!nodes.contains(&from)) {
        from = addNode(from);
    }
    if (!nodes.contains(&to)) {
        to = addNode(to);
    }
    edges[&from].insert(Edge<Node, Label>(from, to, options));
    if (directed) {
        edges[&to].insert(Edge<Node, Label>(to, from, options));
    }
}

template<typename Node, typename Label> requires requires { { std::hash<Label>() } -> std::same_as<unsigned long>; }
std::unordered_set<Node*> Network<Node, Label>::getNeighborhood(const Node& node, const size_t radius,
                                                                const bool center) {
    std::unordered_set<Node*> neighborhood;

    if (!radius) {
        return neighborhood;
    }

    std::unordered_set<Node*> toVisit;
    for (const auto& edge : edges[&node]) {
        if (neighborhood.insert(&edge.to).second) {
            toVisit.insert(&edge.to);
        }
    }

    for (size_t i = 2; i < radius; ++i) {
        if (toVisit.empty()) {
            break;
        }

        std::unordered_set<Node*> nextToVisit;
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
