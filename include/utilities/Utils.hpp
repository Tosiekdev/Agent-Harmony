#pragma once

#include "Concepts.hpp"
#include "GridUtils.hpp"
#include "../space/Point.hpp"

#include <cmath>
#include <concepts>
#include <functional>
#include <utility>
#include <vector>

namespace agh {
/** @brief Normalize integer coordinates into a positive half-open toroidal domain.
 * @param p Input coordinates. @param width Domain width. @param height Domain height.
 * @return Normalized coordinates. @throws std::invalid_argument If dimensions are not positive. */
inline Point convertToToroidal(const Point p, const std::size_t width, const std::size_t height) {
    if (width == 0 || height == 0) throw std::invalid_argument("Spatial dimensions must be positive");
    return checkedPosition(p, width, height, true);
}

/** @brief Discrete layer providing const bounds, dimensions, and toroidal coordinate conversion. */
template<typename T>
concept Grid = requires(const T& t) {
    { t.getWidth() } -> std::same_as<std::size_t>;
    { t.getHeight() } -> std::same_as<std::size_t>;
    { t.outOfBounds(Point{}) } -> std::same_as<bool>;
    { t.isToroidal() } -> std::same_as<bool>;
    { t.toToroidal(Point{}) } -> std::same_as<Point>;
};

/** @brief Map a callback over neighboring cell offsets.
 * Traverses only the Moore square or von Neumann diamond, clipped or wrapped to the domain.
 * @tparam T Discrete grid type. @tparam F Callback type.
 * @param layer Grid being queried. @param pos Center coordinates. @param r Nonnegative radius.
 * @param moore Select Moore rather than von Neumann distance. @param center Include normalized center.
 * @param f Callback mapping each position to a result. @return Collected callback results. */
template<Grid T, std::invocable<Point> F>
auto visitNeighborhood(const T& layer, Point pos, const std::size_t r, const bool moore,
                       const bool center, F&& f) -> std::vector<std::invoke_result_t<F, Point>> {
    const auto width = layer.getWidth(), height = layer.getHeight();
    if (width == 0 || height == 0) throw std::invalid_argument("Spatial dimensions must be positive");
    if (r > (std::numeric_limits<std::size_t>::max() - 1) / 2)
        throw std::length_error("Neighborhood diameter is not representable");
    const bool toroidal = layer.isToroidal();
    if (toroidal) pos = layer.toToroidal(pos);
    std::vector<std::invoke_result_t<F, Point>> result;
    const std::size_t diameter = 2 * r + 1;
    for (std::size_t oy = 0; oy < diameter; ++oy) {
        const auto y = offsetCoordinate(pos.y, oy, r, height, toroidal);
        if (!y) continue;
        for (std::size_t ox = 0; ox < diameter; ++ox) {
            const auto dx = ox > r ? ox - r : r - ox;
            const auto dy = oy > r ? oy - r : r - oy;
            if (!moore && dx + dy > r) continue;
            const auto x = offsetCoordinate(pos.x, ox, r, width, toroidal);
            if (!x) continue;
            const Point p{*x, *y};
            if (!center && p == pos) continue;
            result.push_back(std::invoke(std::forward<F>(f), p));
        }
    }

    return result;
}

/** @brief Accumulate agent results from neighboring cell offsets.
 * @tparam T Discrete grid type. @tparam A Result element type. @tparam F Callback type.
 * @param layer Grid being queried. @param pos Center coordinates. @param r Nonnegative radius.
 * @param moore Select Moore rather than von Neumann distance. @param center Include normalized center.
 * @param f Callback appending each cell's agents. @return Collected agents. */
template<Grid T, typename A, std::invocable<Point, std::vector<A>&> F>
auto visitNeighbors(const T& layer, Point pos, const std::size_t r, const bool moore,
                    const bool center, F&& f) -> std::vector<A> {
    const auto width = layer.getWidth(), height = layer.getHeight();
    if (width == 0 || height == 0) throw std::invalid_argument("Spatial dimensions must be positive");
    if (r > (std::numeric_limits<std::size_t>::max() - 1) / 2)
        throw std::length_error("Neighborhood diameter is not representable");
    const bool toroidal = layer.isToroidal();
    if (toroidal) pos = layer.toToroidal(pos);
    std::vector<A> result;
    const std::size_t diameter = 2 * r + 1;
    for (std::size_t oy = 0; oy < diameter; ++oy) {
        const auto y = offsetCoordinate(pos.y, oy, r, height, toroidal);
        if (!y) continue;
        for (std::size_t ox = 0; ox < diameter; ++ox) {
            const auto dx = ox > r ? ox - r : r - ox;
            const auto dy = oy > r ? oy - r : r - oy;
            if (!moore && dx + dy > r) continue;
            const auto x = offsetCoordinate(pos.x, ox, r, width, toroidal);
            if (!x) continue;
            const Point p{*x, *y};
            if (!center && p == pos) continue;
            std::invoke(std::forward<F>(f), p, result);
        }
    }

    return result;
}

/** @brief Apply a callback to every cell of row-major storage.
 * @tparam T Cell type. @tparam F Callback type.
 * @param grid Storage matching width and height. @param f Cell callback.
 * @param width Positive column count. @param height Positive row count. */
template<typename T, std::invocable<T&> F>
void applyToAll(std::vector<T>& grid, F&& f, const std::size_t width, const std::size_t height) {
    if (grid.empty()) return;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), grid[y * width + x]);
        }
    }
}

/** @brief Apply a position-aware callback to every cell of row-major storage.
 * @tparam T Cell type. @tparam F Callback type.
 * @param grid Storage matching width and height. @param f Position/cell callback.
 * @param width Positive column count. @param height Positive row count. */
template<typename T, std::invocable<Point, T&> F>
void transformAll(std::vector<T>& grid, F&& f, const std::size_t width, const std::size_t height) {
    if (grid.empty()) return;
    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), grid[y * width + x]);
        }
    }
}

/** @brief Compute ordinary Euclidean distance between real coordinates.
 * @param p1 First point. @param p2 Second point. @return Euclidean distance. */
inline float l2(const RealPoint p1, const RealPoint p2) {
    return std::hypot(p2.x - p1.x, p2.y - p1.y);
}

/** @brief Extract the registered position through a non-owning agent pointer. */
inline auto Pos = [](auto a) {
    return *a->pos;
};
}

