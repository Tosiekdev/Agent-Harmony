#pragma once

#include "../utilities/Concepts.hpp"

#include <optional>

namespace agh {
template<Label L>
struct EdgeOptions {
    std::optional<L> label;
    std::optional<double> weight;
};

template<Node N, Label L> requires std::equality_comparable<N> && std::equality_comparable<L>
struct Edge {
    using NodeIter = typename std::list<N>::iterator;

    explicit Edge(NodeIter fromNode, NodeIter toNode) : from(fromNode), to(toNode) {}

    explicit Edge(NodeIter fromNode, NodeIter toNode, EdgeOptions<L> options)
        : from(fromNode), to(toNode), label(std::move(options.label)), weight(std::move(options.weight)) {}

    explicit Edge(NodeIter fromNode, NodeIter toNode, const L& l) : from(fromNode), to(toNode), label(l) {}

    explicit Edge(NodeIter fromNode, NodeIter toNode, const double w) : from(fromNode), to(toNode), weight(w) {}

    explicit Edge(NodeIter fromNode, NodeIter toNode, const L& l, const double w)
        : from(fromNode), to(toNode), label(l), weight(w) {}

    NodeIter from;
    NodeIter to;
    std::optional<L> label;
    std::optional<double> weight;
    bool operator==(const Edge& rhs) const {
        return from == rhs.from && to == rhs.to && label == rhs.label;
    };
};
}

template<agh::Node N, agh::Label L>
struct std::hash<agh::Edge<N, L>> {
    size_t operator()(const agh::Edge<N, L>& e) const {
        return std::hash<N>{}(*e.from) ^ (std::hash<N>{}(*e.to) << 1);
    }
};
