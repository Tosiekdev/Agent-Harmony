#pragma once

#include "Concepts.hpp"
#include "GridUtils.hpp"
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

/** @brief Two sorted disjoint half-open intervals on a grid axis. */
using NeighborhoodRanges = std::array<std::pair<std::size_t, std::size_t>, 2>;

/** @brief Find clipped or wrapped axis intervals without radius arithmetic overflow.
 * @param coordinate Center, normalized for a torus. @param radius Axis radius.
 * @param extent Positive axis extent. @param toroidal Whether to wrap.
 * @return Sorted intervals, with unused intervals empty. */
inline NeighborhoodRanges neighborhoodRanges(std::size_t coordinate, std::size_t radius,
                                            std::size_t extent, bool toroidal) {
    if (!toroidal) {
        const auto low = coordinate > radius ? coordinate - radius : 0;
        const auto high = coordinate >= extent || radius >= extent - 1 - coordinate
                              ? extent : coordinate + radius + 1;
        if (low >= high) return {};
        return {{{low, high}, {0, 0}}};
    }
    if (radius >= extent / 2) return {{{0, extent}, {0, 0}}};
    if (coordinate < radius)
        return {{{0, coordinate + radius + 1}, {extent - (radius - coordinate), extent}}};
    if (radius >= extent - 1 - coordinate)
        return {{{0, radius - (extent - 1 - coordinate)}, {coordinate - radius, extent}}};
    return {{{coordinate - radius, coordinate + radius + 1}, {0, 0}}};
}

/** @brief Reserve a small bounded buffer without speculative large allocations.
 * @tparam A Result type. @tparam T Grid type. @param result Output buffer.
 * @param layer Queried grid. @param radius Query radius. */
template<typename A, Grid T>
void reserveNeighborhood(std::vector<A>& result, const T& layer, std::size_t radius) {
    const auto span = radius >= 32 ? std::size_t{64} : 2 * radius + 1;
    const auto width = std::min(layer.getWidth(), span), height = std::min(layer.getHeight(), span);
    result.reserve(std::min(std::size_t{64}, width * height));
}

/** @brief Visit each local neighboring cell once in row-major order.
 * @tparam T Grid type. @tparam F Point callback. @param layer Queried grid.
 * @param pos Query center. @param radius Query radius. @param moore Select Moore distance.
 * @param center Include the normalized center. @param f Point callback.
 * @throws std::invalid_argument If a grid dimension is zero. */
template<Grid T, std::invocable<Point> F>
void forEachNeighborhoodPoint(const T& layer, Point pos, std::size_t radius,
                              bool moore, bool center, F&& f) {
    const auto width = layer.getWidth(), height = layer.getHeight();
    if (width == 0 || height == 0) throw std::invalid_argument("Spatial dimensions must be positive");
    const bool toroidal = layer.isToroidal();
    if (toroidal) pos = layer.toToroidal(pos);
    const auto ys = neighborhoodRanges(pos.y, radius, height, toroidal);
    const auto xs = neighborhoodRanges(pos.x, radius, width, toroidal);
    for (const auto& [yBegin, yEnd] : ys) for (std::size_t y = yBegin; y < yEnd; ++y) {
        auto dy = y > pos.y ? y - pos.y : pos.y - y;
        if (toroidal) dy = std::min(dy, height - dy);
        if (dy > radius) continue;
        const auto row = moore ? xs : neighborhoodRanges(pos.x, radius - dy, width, toroidal);
        for (const auto& [xBegin, xEnd] : row) for (std::size_t x = xBegin; x < xEnd; ++x) {
            const Point point{x, y};
            if (!center && point == pos) continue;
            std::invoke(std::forward<F>(f), point);
        }
    }
}

/** @brief Map a callback over unique local neighbors in row-major order.
 * @tparam T Grid type. @tparam F Point callback. @param layer Queried grid.
 * @param pos Center, possibly outside a bounded grid. @param r Query radius.
 * @param moore Select Moore distance. @param center Include the normalized center.
 * @param f Point callback. @return Callback results. */
template<Grid T, std::invocable<Point> F>
auto visitNeighborhood(const T& layer, Point pos, std::size_t r, bool moore,
                       bool center, F&& f) -> std::vector<std::invoke_result_t<F, Point>> {
    std::vector<std::invoke_result_t<F, Point>> result;
    reserveNeighborhood(result, layer, r);
    forEachNeighborhoodPoint(layer, pos, r, moore, center,
        [&](Point point) { result.push_back(std::invoke(std::forward<F>(f), point)); });
    return result;
}

/** @brief Accumulate agents from unique local neighboring cells.
 * @tparam T Grid type. @tparam A Result type. @tparam F Accumulation callback.
 * @param layer Queried grid. @param pos Query center. @param r Query radius.
 * @param moore Select Moore distance. @param center Include the normalized center.
 * @param f Accumulation callback. @return Collected results. */
template<Grid T, typename A, std::invocable<Point, std::vector<A>&> F>
auto visitNeighbors(const T& layer, Point pos, std::size_t r, bool moore,
                    bool center, F&& f) -> std::vector<A> {
    std::vector<A> result;
    reserveNeighborhood(result, layer, r);
    forEachNeighborhoodPoint(layer, pos, r, moore, center,
        [&](Point point) { std::invoke(std::forward<F>(f), point, result); });
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

