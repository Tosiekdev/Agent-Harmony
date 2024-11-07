#pragma once

#include "../Concepts.hpp"

#include <optional>

namespace abmf {
template<Label L>
struct EdgeOptions {
    std::optional<L> label;
    std::optional<double> weight;
};

template<Node N, Label L> requires std::equality_comparable<N> && std::equality_comparable<L>
struct Edge {
    explicit Edge(const size_t fromNode, const size_t toNode) : from(fromNode), to(toNode) {}

    explicit Edge(const size_t fromNode, const size_t toNode, EdgeOptions<L> options)
        : from(fromNode), to(toNode), label(std::move(options.label)), weight(std::move(options.weight)) {}

    explicit Edge(const size_t fromNode, const size_t toNode, const L& l) : from(fromNode), to(toNode), label(l) {}

    explicit Edge(const size_t fromNode, const size_t toNode, const double w) : from(fromNode), to(toNode), weight(w) {}

    explicit Edge(const size_t fromNode, const size_t toNode, L const& l, const double w)
        : from(fromNode), to(toNode), label(l), weight(w) {}

    N& from;
    N& to;
    std::optional<L> label;
    std::optional<double> weight;
    bool operator==(const Edge&) const = default;
};
}
