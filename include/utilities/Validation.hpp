#pragma once

#include "../space/Point.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace agh::detail {
inline void validateDimensions(int width, int height) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Spatial dimensions must be positive");
}

template<typename Cell>
std::size_t checkedCellCount(int width, int height) {
    validateDimensions(width, height);
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    if (w > std::vector<Cell>().max_size() / h)
        throw std::length_error("Spatial storage size is not representable");
    return w * h;
}

inline int checkedBucketCount(float extent, float dx) {
    if (!std::isfinite(extent) || extent <= 0 || !std::isfinite(dx) || dx <= 0)
        throw std::invalid_argument("Extent and discretization must be finite and positive");
    const double count = std::ceil(static_cast<double>(extent) / dx);
    if (count > std::numeric_limits<int>::max())
        throw std::length_error("Bucket count is not representable");
    return static_cast<int>(count);
}

inline int wrap(int value, int extent) {
    const int remainder = value % extent;
    return remainder < 0 ? remainder + extent : remainder;
}

inline Point checkedPosition(Point pos, int width, int height, bool toroidal) {
    validateDimensions(width, height);
    if (toroidal) return {wrap(pos.x, width), wrap(pos.y, height)};
    if (pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height)
        throw std::out_of_range("Position lies outside the spatial domain");
    return pos;
}

inline RealPoint checkedPosition(RealPoint pos, float width, float height, bool toroidal) {
    if (!std::isfinite(pos.x) || !std::isfinite(pos.y))
        throw std::invalid_argument("Coordinates must be finite");
    if (toroidal) {
        auto wrapReal = [](float value, float extent) {
            double result = std::fmod(static_cast<double>(value), static_cast<double>(extent));
            if (result < 0) result += extent;
            const float normalized = static_cast<float>(result);
            return normalized >= extent ? 0.0f : normalized;
        };
        return {wrapReal(pos.x, width), wrapReal(pos.y, height)};
    }
    if (pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height)
        throw std::out_of_range("Position lies outside the spatial domain");
    return pos;
}

inline std::size_t checkedIndex(Point pos, int width, int height, bool toroidal = false) {
    pos = checkedPosition(pos, width, height, toroidal);
    return static_cast<std::size_t>(pos.y) * static_cast<std::size_t>(width) + pos.x;
}

inline void validateRadius(float radius) {
    if (!std::isfinite(radius) || radius < 0)
        throw std::invalid_argument("Radius must be finite and nonnegative");
}
}
