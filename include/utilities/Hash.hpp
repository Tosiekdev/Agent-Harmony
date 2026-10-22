#pragma once

#include "Concepts.hpp"

#include <list>

namespace agh {
    /** @brief Hash a live read-only node iterator using its node value. @tparam N Node type. */
    template<Node N>
    struct Hash {
        /** @brief Compute the node hash. @param n Live iterator. @return Hash value. */
        size_t operator()(const typename std::list<N>::const_iterator& n) const noexcept {
            return std::hash<N>{}(*n);
        }
    };
}