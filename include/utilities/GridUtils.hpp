#pragma once

#include "../space/Point.hpp"
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>

namespace agh {
/** @brief Restore a traversal flag on normal or exceptional callback exit. */
class TraversalGuard {
    bool& traversing;
    bool previous;
public:
    /** @brief Mark a traversal active. @param flag Flag owned by the traversed space. */
    explicit TraversalGuard(bool& flag) : traversing(flag), previous(flag) { flag = true; }
    /** @brief Restore the state preceding this traversal, including nested calls. */
    ~TraversalGuard() { traversing = previous; }
    /** @brief A guard cannot be copied. @param other Source guard. */
    TraversalGuard(const TraversalGuard& other) = delete;
    /** @brief A guard cannot be assigned. @param other Source guard. */
    TraversalGuard& operator=(const TraversalGuard& other) = delete;
};

/** @brief Reject changes that invalidate an active traversal.
 * @param traversing Current traversal flag. @throws std::logic_error If traversal is active. */
inline void requireStructuralMutation(bool traversing) {
    if (traversing) throw std::logic_error("Defer membership changes until traversal finishes");
}

/** @brief Compute a positive grid's cell count without unsigned multiplication overflow.
 * @tparam Cell Stored cell type. @param width Column count. @param height Row count.
 * @return Cell count. @throws std::invalid_argument If a dimension is zero.
 * @throws std::length_error If the product is not representable. */
template<typename Cell>
std::size_t checkedCellCount(std::size_t width, std::size_t height) {
    if (width == 0 || height == 0) throw std::invalid_argument("Spatial dimensions must be positive");
    if (width > std::numeric_limits<std::size_t>::max() / height)
        throw std::length_error("Spatial storage size is not representable");
    return width * height;
}

/** @brief Convert a continuous extent to a representable bucket count.
 * @param extent Positive finite extent. @param dx Positive finite bucket size.
 * @return Rounded-up bucket count. @throws std::invalid_argument If inputs are not positive and finite.
 * @throws std::length_error If conversion would exceed the count type. */
inline std::size_t checkedBucketCount(float extent, float dx) {
    if (!std::isfinite(extent) || extent <= 0 || !std::isfinite(dx) || dx <= 0)
        throw std::invalid_argument("Extent and discretization must be finite and positive");
    const double count = std::ceil(static_cast<double>(extent) / dx);
    if (count >= static_cast<double>(std::numeric_limits<std::size_t>::max()))
        throw std::length_error("Bucket count is not representable");
    return static_cast<std::size_t>(count);
}

/** @brief Normalize unsigned discrete coordinates or validate bounded access.
 * @param pos Input point. @param width Positive columns. @param height Positive rows.
 * @param toroidal Whether to wrap. @return In-domain point.
 * @throws std::out_of_range If bounded coordinates exceed the domain. */
inline Point checkedPosition(Point pos, std::size_t width, std::size_t height, bool toroidal) {
    if (toroidal) return {pos.x % width, pos.y % height};
    if (pos.x >= width || pos.y >= height) throw std::out_of_range("Position lies outside the spatial domain");
    return pos;
}

/** @brief Normalize continuous coordinates or validate bounded access before bucket conversion.
 * @param pos Input point. @param width Positive finite width. @param height Positive finite height.
 * @param toroidal Whether to wrap. @return In-domain point.
 * @throws std::invalid_argument If coordinates are not finite.
 * @throws std::out_of_range If bounded coordinates lie outside the domain. */
inline RealPoint checkedPosition(RealPoint pos, float width, float height, bool toroidal) {
    if (!std::isfinite(pos.x) || !std::isfinite(pos.y)) throw std::invalid_argument("Coordinates must be finite");
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

/** @brief Produce a row-major index from validated grid coordinates.
 * @param pos Input point. @param width Columns. @param height Rows. @param toroidal Whether to wrap.
 * @return Storage index. @throws std::out_of_range If bounded coordinates exceed the domain. */
inline std::size_t checkedIndex(Point pos, std::size_t width, std::size_t height, bool toroidal = false) {
    pos = checkedPosition(pos, width, height, toroidal);
    return pos.y * width + pos.x;
}

/** @brief Validate a continuous query radius before candidate bucket conversion.
 * @param radius Query radius. @throws std::invalid_argument If negative or not finite. */
inline void validateRadius(float radius) {
    if (!std::isfinite(radius) || radius < 0) throw std::invalid_argument("Radius must be finite and nonnegative");
}

/** @brief Apply a neighborhood offset without unsigned underflow or overflow.
 * @param coordinate Center, already normalized for a torus. @param offset Offset in the diameter.
 * @param radius Radius defining the zero offset. @param extent Positive axis extent.
 * @param toroidal Whether to wrap. @return An in-domain coordinate, or no value when clipped. */
inline std::optional<std::size_t> offsetCoordinate(std::size_t coordinate, std::size_t offset,
                                                 std::size_t radius, std::size_t extent, bool toroidal) {
    if (offset < radius) {
        auto shift = radius - offset;
        if (toroidal) {
            shift %= extent;
            return coordinate >= shift ? coordinate - shift : extent - (shift - coordinate);
        }
        if (shift > coordinate) return std::nullopt;
        coordinate -= shift;
    } else {
        auto shift = offset - radius;
        if (toroidal) {
            shift %= extent;
            return shift >= extent - coordinate ? shift - (extent - coordinate) : coordinate + shift;
        }
        if (shift > std::numeric_limits<std::size_t>::max() - coordinate) return std::nullopt;
        coordinate += shift;
    }
    return coordinate < extent ? std::optional{coordinate} : std::nullopt;
}
}
