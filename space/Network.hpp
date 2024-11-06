#pragma once

#include "Edge.hpp"

#include <list>
#include <unordered_map>
#include <unordered_set>

namespace abmf {
template<typename Node, typename Label> requires requires {
    { std::hash<Label>() } -> std::same_as<size_t>;
}
class Network {
public:
    explicit Network(const bool isDirected = false) : directed(isDirected) {};

    void addNode(const Node& node);

private:
    std::list<Node> nodes;
    std::unordered_map<Node*, std::unordered_set<Edge<Label>>> labels;
    bool directed;
};
}

#include "NetworkImpl.hpp"
