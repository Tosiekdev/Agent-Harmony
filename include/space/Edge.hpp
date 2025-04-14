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
    explicit Edge(N& fromNode, N& toNode) : from(fromNode), to(toNode) {}

    explicit Edge(N& fromNode, N& toNode, EdgeOptions<L> options)
        : from(fromNode), to(toNode), label(std::move(options.label)), weight(std::move(options.weight)) {}

    explicit Edge(N& fromNode, N& toNode, const L& l) : from(fromNode), to(toNode), label(l) {}

    explicit Edge(N& fromNode, N& toNode, const double w) : from(fromNode), to(toNode), weight(w) {}

    explicit Edge(N& fromNode, N& toNode, const L& l, const double w)
        : from(fromNode), to(toNode), label(l), weight(w) {}

    N& from;
    N& to;
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
        return std::hash<N>{}(e.from) ^ (std::hash<N>{}(e.to) << 1);
    }
};
