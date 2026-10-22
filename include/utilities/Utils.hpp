#pragma once

#include "Concepts.hpp"
#include "Validation.hpp"
#include "../space/Point.hpp"

#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <utility>
#include <vector>

namespace agh {
/** @brief Normalize integer coordinates into a positive half-open toroidal domain.
 * @param p Input coordinates. @param width Domain width. @param height Domain height.
 * @return Normalized coordinates. @throws std::invalid_argument If dimensions are not positive. */
inline Point convertToToroidal(const Point p, const int width, const int height) {
    return detail::checkedPosition(p, width, height, true);

}

/** @brief Discrete layer providing const bounds, dimensions, and toroidal coordinate conversion. */
template<typename T>
concept Grid = requires(const T& t) {
    { t.getWidth() } -> std::same_as<int>;
    { t.getHeight() } -> std::same_as<int>;
    { t.outOfBounds(Point{}) } -> std::same_as<bool>;
    { t.isToroidal() } -> std::same_as<bool>;
    { t.toToroidal(Point{}) } -> std::same_as<Point>;
};

namespace detail {
template<typename A, Grid T>
void reserveNeighborhood(std::vector<A>& result, const T& layer, int radius) {
    if (radius < 0) return; // The traversal reports invalid query parameters.
    const long long diameter = 2LL * radius + 1;
    const auto width = std::min<long long>(layer.getWidth(), diameter);
    const auto height = std::min<long long>(layer.getHeight(), diameter);
    // Preallocate small neighborhoods without requesting huge speculative buffers.
    if (width > 0 && height > 0)
        result.reserve(std::min(result.max_size(), static_cast<std::size_t>(std::min(64LL, width * height))));
}

using NeighborhoodRanges = std::array<std::pair<int, int>, 2>;

// Sorted, disjoint half-open intervals preserve row-major order across a seam.
inline NeighborhoodRanges neighborhoodRanges(int coordinate, long long radius, int extent, bool toroidal) {
    const long long low = coordinate - radius, high = coordinate + radius + 1;
    if (!toroidal)
        return {{{static_cast<int>(std::max(0LL, low)),
                  static_cast<int>(std::min<long long>(extent, high))}, {0, 0}}};
    if (radius >= extent / 2) return {{{0, extent}, {0, 0}}};
    if (low < 0)
        return {{{0, static_cast<int>(high)}, {static_cast<int>(extent + low), extent}}};
    if (high > extent)
        return {{{0, static_cast<int>(high - extent)}, {static_cast<int>(low), extent}}};
    return {{{static_cast<int>(low), static_cast<int>(high)}, {0, 0}}};
}

template<Grid T, std::invocable<Point> F>
void forEachNeighborhoodPoint(const T& layer, Point pos, int radius, bool moore, bool center, F&& f) {
    if (radius < 0) throw std::invalid_argument("Radius must be nonnegative");
    const int width = layer.getWidth(), height = layer.getHeight();
    const bool toroidal = layer.isToroidal();
    pos = checkedPosition(pos, width, height, toroidal);
    const auto ys = neighborhoodRanges(pos.y, radius, height, toroidal);
    const auto xs = neighborhoodRanges(pos.x, radius, width, toroidal);
    for (const auto& [yBegin, yEnd] : ys) {
        for (int y = yBegin; y < yEnd; ++y) {
            int dy = std::abs(y - pos.y);
            if (toroidal) dy = std::min(dy, height - dy);
            const auto row = moore ? xs : neighborhoodRanges(pos.x, static_cast<long long>(radius) - dy, width, toroidal);
            for (const auto& [xBegin, xEnd] : row) {
                for (int x = xBegin; x < xEnd; ++x) {
                    const Point point{x, y};
                    if (!center && point == pos) continue;
                    std::invoke(f, point);
                }
            }
        }
    }
}
}

/** @brief Map a callback over unique neighboring cells in row-major order.
 * Traverses only the Moore square or von Neumann diamond, clipped or wrapped to the domain.
 * @tparam T Discrete grid type. @tparam F Callback type.
 * @param layer Grid being queried. @param pos Center coordinates. @param r Nonnegative radius.
 * @param moore Select Moore rather than von Neumann distance. @param center Include normalized center.
 * @param f Callback mapping each position to a result. @return Collected callback results. */
template<Grid T, std::invocable<Point> F>
auto visitNeighborhood(const T& layer, Point pos, int r, bool moore,
                       bool center, F&& f) -> std::vector<std::invoke_result_t<F, Point>> {
    std::vector<std::invoke_result_t<F, Point>> result;
    detail::reserveNeighborhood(result, layer, r);
    detail::forEachNeighborhoodPoint(layer, pos, r, moore, center,
        [&](Point point) { result.push_back(std::invoke(std::forward<F>(f), point)); });
    return result;
}

/** @brief Accumulate agent results from unique neighboring cells.
 * @tparam T Discrete grid type. @tparam A Result element type. @tparam F Callback type.
 * @param layer Grid being queried. @param pos Center coordinates. @param r Nonnegative radius.
 * @param moore Select Moore rather than von Neumann distance. @param center Include normalized center.
 * @param f Callback appending each cell's agents. @return Collected agents. */
template<Grid T, typename A, std::invocable<Point, std::vector<A>&> F>
auto visitNeighbors(const T& layer, Point pos, int r, bool moore,
                    bool center, F&& f) -> std::vector<A> {
    std::vector<A> result;
    detail::reserveNeighborhood(result, layer, r);
    detail::forEachNeighborhoodPoint(layer, pos, r, moore, center,
        [&](Point point) { std::invoke(std::forward<F>(f), point, result); });
    return result;
}

/** @brief Apply a callback to every cell of row-major storage.
 * @tparam T Cell type. @tparam F Callback type.
 * @param grid Storage matching width and height. @param f Cell callback.
 * @param width Positive column count. @param height Positive row count. */
template<typename T, std::invocable<T&> F>
void applyToAll(std::vector<T>& grid, F&& f, const int width, const int height) {
    if (grid.empty()) return;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), grid[static_cast<std::size_t>(y) * width + x]);
        }
    }
}

/** @brief Apply a position-aware callback to every cell of row-major storage.
 * @tparam T Cell type. @tparam F Callback type.
 * @param grid Storage matching width and height. @param f Position/cell callback.
 * @param width Positive column count. @param height Positive row count. */
template<typename T, std::invocable<Point, T&> F>
void transformAll(std::vector<T>& grid, F&& f, const int width, const int height) {
    if (grid.empty()) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), grid[static_cast<std::size_t>(y) * width + x]);
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

