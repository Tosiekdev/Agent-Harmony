#pragma once

#include <optional>

namespace abmf {
template<typename Label>
struct Edge {
    explicit Edge(const size_t fromNode, const size_t toNode) : from(fromNode), to(toNode) {}

    explicit Edge(const size_t fromNode, const size_t toNode, const Label& l) : from(fromNode), to(toNode), label(l) {}

    explicit Edge(const size_t fromNode, const size_t toNode, const double w) : from(fromNode), to(toNode), weight(w) {}

    explicit Edge(const size_t fromNode, const size_t toNode, Label const& l, const double w)
        : from(fromNode), to(toNode), label(l), weight(w) {}

    size_t from;
    size_t to;
    std::optional<Label> label;
    std::optional<double> weight;
};
}
