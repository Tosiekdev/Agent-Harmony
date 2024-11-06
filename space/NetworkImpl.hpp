#pragma once

#include "Network.hpp"

namespace abmf {
template<typename Node, typename Label> requires requires { { std::hash<Label>() } -> std::same_as<unsigned long>; }
void Network<Node, Label>::addNode(const Node& node) {
    nodes.emplace_back(node);
}
}
