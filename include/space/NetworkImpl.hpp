#pragma once

#include "Network.hpp"

#include <algorithm>
#include <queue>

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
            std::erase_if(edges[n], [&node](const auto& e) { return e.to == node; });
        }
    }
    else {
        for (auto& [n, e] : edges) {
            std::erase_if(e, [&node](const auto& edge) { return edge.to == node; });
        }
    }
    edges.erase(it->second);
    nodes.remove(node);
}

template<Node N, Label L>
const typename Network<N, L>::EdgeSet& Network<N, L>::getEdges(const N& node) const {
    auto it = lookup.find(node);
    return edges.at(it->second);
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
void Network<N, L>::updateNode(const N& node) {
    auto it = lookup.find(node);
    if (it == lookup.end()) {
        return;
    }
    *it->second = node;
}

template<Node N, Label L>
void Network<N, L>::removeEdge(const N& from, const N& to) {
    N* fromPtr = lookup[from];
    N* toPtr = lookup[to];
    removeEdge(Edge<N, L>(*fromPtr, *toPtr));
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
            std::erase_if(edges[n], [&node](const auto& e) { return e.to == node; });
        }
    }
    else {
        for (auto& [n, e] : edges) {
            std::erase_if(e, [&node](const auto& edge) { return edge.to == node; });
        }
    }
    edges[it->second].clear();
}

template<Node N, Label L>
bool Network<N, L>::hasNode(const N& node) {
    return std::find(nodes.begin(), nodes.end(), node) != nodes.end();
}

template<Node N, Label L>
std::vector<N*> Network<N, L>::getNeighborhood(const N& node, const size_t radius,
                                               const bool center) {
    struct NodeHelper {
        N* node;
        size_t distance;
    };

    auto it = lookup.find(node);
    if (it == lookup.end()) {
        return {};
    }

    if (!radius) {
        return center ? std::vector<N*>{it->second} : std::vector<N*>{};
    }

    std::vector<N*> result;
    std::queue<NodeHelper> q;
    std::unordered_set<N*> visited;
    q.push(NodeHelper{it->second, 0});
    visited.insert(it->second);

    while (!q.empty()) {
        auto u = q.front();
        q.pop();
        if (u.distance == radius) {
            continue;
        }
        std::for_each(edges[u.node].begin(),
                      edges[u.node].end(),
                      [&](const auto& edge) {
                          if (visited.contains(&edge.to)) {
                              return;
                          }
                          visited.insert(&edge.to);
                          result.emplace_back(&edge.to);
                          q.push(NodeHelper{&edge.to, u.distance + 1});
                      });
    }

    if (center) {
        result.push_back(it->second);
    }

    return result;
}
}
