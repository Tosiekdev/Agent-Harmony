#pragma once

#include "Network.hpp"

#include <algorithm>

namespace abmf {
template<Node N, Label L>
N& Network<N, L>::addNode(const N& node) {
    if (auto it = std::find(nodes.begin(), nodes.end(), node); it != nodes.end()) {
        return *it;
    }
    nodes.push_back(node);
    auto& newNode = nodes.back();
    edges.insert({&newNode, EdgeSet()});
    return newNode;
}

template<Node N, Label L>
void Network<N, L>::deleteNode(N& node) {
    if (!directed) {
        std::unordered_set<N*> nghs;
        for (const auto& e : edges[&node]) {
            nghs.insert(&e.to);
        }
        for (auto n : nghs) {
            std::erase_if(edges[n], [&](const auto& e){ return e.to == node;});
        }
    } else {
        for (auto& [n, e] : edges) {
            std::erase_if(e, [&](const auto& edge){ return edge.to == node;});
        }
    }
    edges.erase(&node);
    auto it = std::find(nodes.begin(), nodes.end(), node);
    nodes.erase(it);
}

template<Node N, Label L>
void Network<N, L>::addEdge(N& from, N& to) {
    N* f = nullptr;
    N* t = nullptr;
    if (auto it = std::find(nodes.begin(), nodes.end(), from); it != nodes.end()) {
        f = &*it;
    } else {
        f = &addNode(from);
    }
    if (auto it = std::find(nodes.begin(), nodes.end(), to); it != nodes.end()) {
        t = &*it;
    } else {
        t = &addNode(to);
    }

    edges[f].insert(Edge<N, L>(*f, *t));
    if (directed) {
        edges[t].insert(Edge<N, L>(*t, *f));
    }
}

template<Node N, Label L>
void Network<N, L>::addEdge(N& from, N& to, EdgeOptions<L> options) {
    N* f = nullptr;
    N* t = nullptr;
    if (auto it = std::find(nodes.begin(), nodes.end(), from); it != nodes.end()) {
        f = &*it;
    } else {
        f = &addNode(from);
    }
    if (auto it = std::find(nodes.begin(), nodes.end(), to); it != nodes.end()) {
        t = &*it;
    } else {
        t = &addNode(to);
    }

    edges[f].insert(Edge<N, L>(*f, *t, options));
    if (directed) {
        edges[t].insert(Edge<N, L>(*t, *f, options));
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
    if (!directed) {
        std::unordered_set<N*> nghs;
        for (const auto& e : edges[&node]) {
            nghs.insert(&e.to);
        }
        for (auto n : nghs) {
            std::erase_if(edges[n], [&](const auto& e){ return e.to == node;});
        }
    } else {
        for (auto& [n, e] : edges) {
            std::erase_if(e, [&](const auto& edge){ return edge.to == node;});
        }
    }
    edges[&node].clear();
}

template<Node N, Label L>
bool Network<N, L>::hasNode(const N& node) {
    return std::find(nodes.begin(), nodes.end(), node) != nodes.end();
}

template<Node N, Label L>
std::unordered_set<N*> Network<N, L>::getNeighborhood(const N& node, const size_t radius,
                                                                const bool center) {
    std::unordered_set<N*> neighborhood;

    if (!radius) {
        return neighborhood;
    }

    std::unordered_set<N*> toVisit;
    for (const auto& edge : edges[&node]) {
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
        neighborhood.insert(&node);
    }

    return neighborhood;
}
}
