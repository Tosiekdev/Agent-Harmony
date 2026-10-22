#pragma once

#include "../utilities/Concepts.hpp"

#include <optional>

namespace agh {
/** @brief Optional edge label and weight. @tparam L Hashable, equality-comparable label type. */
template<Label L>
struct EdgeOptions {
    /** @brief Edge label, or no label for an unlabeled edge. */
    std::optional<L> label;
    /** @brief Optional numerical edge weight. */
    std::optional<double> weight;
};

/** @brief Non-owning edge between live read-only node iterators from one network.
 * @tparam N Node type. @tparam L Label type. */
template<Node N, Label L> requires std::equality_comparable<N> && std::equality_comparable<L>
struct Edge {
    /** @brief Iterator to a read-only network node. */
    using NodeIter = typename std::list<N>::const_iterator;

    /** @brief Construct an unlabeled, unweighted edge.
     * @param fromNode Source iterator. @param toNode Destination iterator. */
    explicit Edge(NodeIter fromNode, NodeIter toNode) : from(fromNode), to(toNode) {}

    /** @brief Construct an edge from options.
     * @param fromNode Source iterator. @param toNode Destination iterator. @param options Label and weight. */
    explicit Edge(NodeIter fromNode, NodeIter toNode, EdgeOptions<L> options)
        : from(fromNode), to(toNode), label(std::move(options.label)), weight(std::move(options.weight)) {}

    /** @brief Construct a labeled edge.
     * @param fromNode Source iterator. @param toNode Destination iterator. @param l Label. */
    explicit Edge(NodeIter fromNode, NodeIter toNode, const L& l) : from(fromNode), to(toNode), label(l) {}

    /** @brief Construct a weighted edge.
     * @param fromNode Source iterator. @param toNode Destination iterator. @param w Weight. */
    explicit Edge(NodeIter fromNode, NodeIter toNode, const double w) : from(fromNode), to(toNode), weight(w) {}

    /** @brief Construct a labeled, weighted edge.
     * @param fromNode Source iterator. @param toNode Destination iterator. @param l Label. @param w Weight. */
    explicit Edge(NodeIter fromNode, NodeIter toNode, const L& l, const double w)
        : from(fromNode), to(toNode), label(l), weight(w) {}

    /** @brief Source node iterator. */
    NodeIter from;
    /** @brief Destination node iterator. */
    NodeIter to;
    /** @brief Edge label, or no label for an unlabeled edge. */
    std::optional<L> label;
    /** @brief Optional numerical edge weight. */
    std::optional<double> weight;
    /** @brief Compare endpoints and label; weight does not determine edge identity. @param rhs Edge to compare. */
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
