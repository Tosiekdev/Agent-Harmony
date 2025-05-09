#pragma once

#include "Concepts.hpp"

#include <list>

namespace agh {
    template<Node N>
    struct Hash {
        size_t operator()(const typename std::list<N>::iterator& n) const noexcept {
            return std::hash<N>{}(*n);
        }
    };
}