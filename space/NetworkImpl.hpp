#pragma once

#include "Network.hpp"

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
}
