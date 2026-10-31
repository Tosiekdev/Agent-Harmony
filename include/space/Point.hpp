#pragma once

#include <cstddef>

namespace agh {
/** @brief Integer coordinates of a discrete cell. */
struct Point {
    /** @brief Horizontal coordinate. */
    std::size_t x{};
    /** @brief Vertical coordinate. */
    std::size_t y{};

    /** @brief Compare both coordinates exactly. @param other Position to compare. */
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

/** @brief Floating-point coordinates of a continuous-space position. */
struct RealPoint {
    /** @brief Horizontal coordinate. */
    float x{};
    /** @brief Vertical coordinate. */
    float y{};

    /** @brief Compare both coordinates exactly. @param other Position to compare. */
    bool operator==(const RealPoint& other) const {
        return x == other.x && y == other.y;
    }
};
}
