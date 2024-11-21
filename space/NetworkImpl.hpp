#pragma once

#include "Network.hpp"

#include <algorithm>

namespace abmf {
template<Node N, Label L>
N& Network<N, L>::addNode(const N& node) {
    if (auto it = lookup.find(node); it != lookup.end()) {
        return *it->second;
    }
    nodes.push_back(node);
    auto& newNode = nodes.back();
    edges.insert({&newNode, EdgeSet()});
    lookup[node] = &newNode;
    return newNode;
}

template<Node N, Label L>
void Network<N, L>::deleteNode(N& node) {
    auto it = lookup.find(node);
    if (it == lookup.end()) {
        return;
    }
    if (!directed) {
        std::unordered_set<N*> nghs;
        for (const auto& e : edges[it->second]) {
            nghs.insert(&e.to);
        }
        for (auto n : nghs) {
            std::erase_if(edges[n], [&node](const auto& e){ return e.to == node;});
        }
    } else {
        for (auto& [n, e] : edges) {
            std::erase_if(e, [&node](const auto& edge){ return edge.to == node;});
        }
    }
    edges.erase(it->second);
    auto i = std::find(nodes.begin(), nodes.end(), node);
    nodes.erase(i);
}

template<Node N, Label L>
void Network<N, L>::addEdge(const N& from, const N& to) {
    N& f = addNode(from);
    N& t = addNode(to);

    edges[&f].insert(Edge<N, L>(f, t));
    if (directed) {
        edges[&t].insert(Edge<N, L>(t, f));
    }
}

template<Node N, Label L>
void Network<N, L>::addEdge(const N& from, const N& to, EdgeOptions<L> options) {
    N& f = addNode(from);
    N& t = addNode(to);

    edges[&f].insert(Edge<N, L>(f, t, options));
    if (directed) {
        edges[&t].insert(Edge<N, L>(t, f, options));
    }
}

template<Node N, Label L>
void Network<N, L>::removeEdge(const EdgeT& edge) {
    edges[&edge.from].erase(edge);
    if (directed) {
        edges[&edge.to].erase(Edge(edge.to, edge.from, EdgeOptions<L>{edge.label, edge.weight}));
    }
}

template<Node N, Label L>
void Network<N, L>::removeEdges(const N& node) {
    auto it = lookup.find(node);
    if (it == lookup.end()) {
        return;
    }
    if (!directed) {
        std::unordered_set<N*> nghs;
        for (const auto& e : edges[it->second]) {
            nghs.insert(&e.to);
        }
        for (auto n : nghs) {
            std::erase_if(edges[n], [&node](const auto& e){ return e.to == node;});
        }
    } else {
        for (auto& [n, e] : edges) {
            std::erase_if(e, [&node](const auto& edge){ return edge.to == node;});
        }
    }
    edges[it->second].clear();
}

template<Node N, Label L>
bool Network<N, L>::hasNode(const N& node) {
    return std::find(nodes.begin(), nodes.end(), node) != nodes.end();
}

template<Node N, Label L>
std::unordered_set<N*> Network<N, L>::getNeighborhood(const N& node, const size_t radius,
                                                                const bool center) {
    auto it = lookup.find(node);
    if (it == lookup.end()) {
        return {};
    }
    std::unordered_set<N*> neighborhood;

    if (!radius) {
        return neighborhood;
    }

    std::unordered_set<N*> toVisit;
    for (const auto& edge : edges[it->second]) {
        if (neighborhood.insert(&edge.to).second) {
            toVisit.insert(&edge.to);
        }
    }

    for (size_t i = 2; i < radius; ++i) {
        if (toVisit.empty()) {
            break;
        }

        std::unordered_set<N*> nextToVisit;
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
        neighborhood.insert(it->second);
    }

    return neighborhood;
}
}
