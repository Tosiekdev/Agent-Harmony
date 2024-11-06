#pragma once

#include <optional>

namespace abmf {
template<typename Label>
struct EdgeOptions {
    std::optional<Label> label;
    std::optional<double> weight;
};

template<typename Node, typename Label>
struct Edge {
    explicit Edge(const size_t fromNode, const size_t toNode) : from(fromNode), to(toNode) {}

    explicit Edge(const size_t fromNode, const size_t toNode, EdgeOptions<Label> options)
        : from(fromNode), to(toNode), label(std::move(options.label)), weight(std::move(options.weight)) {}

    explicit Edge(const size_t fromNode, const size_t toNode, const Label& l) : from(fromNode), to(toNode), label(l) {}

    explicit Edge(const size_t fromNode, const size_t toNode, const double w) : from(fromNode), to(toNode), weight(w) {}

    explicit Edge(const size_t fromNode, const size_t toNode, Label const& l, const double w)
        : from(fromNode), to(toNode), label(l), weight(w) {}

    Node& from;
    Node& to;
    std::optional<Label> label;
    std::optional<double> weight;
};
}
