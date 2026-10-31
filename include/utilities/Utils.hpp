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

/** @brief Map a callback over neighboring cell offsets.
 * Traverses only the Moore square or von Neumann diamond, clipped or wrapped to the domain.
 * @tparam T Discrete grid type. @tparam F Callback type.
 * @param layer Grid being queried. @param pos Center coordinates. @param r Nonnegative radius.
 * @param moore Select Moore rather than von Neumann distance. @param center Include normalized center.
 * @param f Callback mapping each position to a result. @return Collected callback results. */
template<Grid T, std::invocable<Point> F>
auto visitNeighborhood(const T& layer, Point pos, const int r, const bool moore,
                       const bool center, F&& f) -> std::vector<std::invoke_result_t<F, Point>> {
    if (r < 0) throw std::invalid_argument("Radius must be nonnegative");
    if (r > (std::numeric_limits<int>::max() - 1) / 2)
        throw std::length_error("Neighborhood offset is not representable");
    if (layer.isToroidal()) pos = layer.toToroidal(pos);
    else if (layer.outOfBounds(pos)) throw std::out_of_range("Neighborhood center lies outside the domain");
    std::vector<std::invoke_result_t<F, Point>> result;
    if (moore) {
        result.reserve((std::size_t(2) * r + 1) * (std::size_t(2) * r + 1));
    }
    else {
        result.reserve(std::size_t(r) * r + (std::size_t(r) + 1) * (std::size_t(r) + 1));
    }
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (!moore && std::abs(dx) + std::abs(dy) > r) continue;

            const auto px = static_cast<long long>(pos.x) + dx;
            const auto py = static_cast<long long>(pos.y) + dy;
            Point p;
            if (layer.isToroidal()) {
                p = {static_cast<int>(px % layer.getWidth()), static_cast<int>(py % layer.getHeight())};
                p = layer.toToroidal(p);
            } else {
                if (px < 0 || py < 0 || px >= layer.getWidth() || py >= layer.getHeight()) continue;
                p = {static_cast<int>(px), static_cast<int>(py)};
            }

            if (dx == 0 && dy == 0 && !center) continue;

            if (layer.outOfBounds(p)) {
                if (layer.isToroidal()) {
                    p = layer.toToroidal(p);
                    result.push_back(std::invoke(std::forward<F>(f), p));
                }
            }
            else {
                result.push_back(std::invoke(std::forward<F>(f), p));
            }
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
auto visitNeighbors(const T& layer, Point pos, const int r, const bool moore,
                    const bool center, F&& f) -> std::vector<A> {
    if (r < 0) throw std::invalid_argument("Radius must be nonnegative");
    if (r > (std::numeric_limits<int>::max() - 1) / 2)
        throw std::length_error("Neighborhood offset is not representable");
    if (layer.isToroidal()) pos = layer.toToroidal(pos);
    else if (layer.outOfBounds(pos)) throw std::out_of_range("Neighborhood center lies outside the domain");
    std::vector<A> result;
    if (moore) {
        result.reserve((std::size_t(2) * r + 1) * (std::size_t(2) * r + 1));
    }
    else {
        result.reserve(std::size_t(r) * r + (std::size_t(r) + 1) * (std::size_t(r) + 1));
    }
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (!moore && std::abs(dx) + std::abs(dy) > r) continue;

            const auto px = static_cast<long long>(pos.x) + dx;
            const auto py = static_cast<long long>(pos.y) + dy;
            Point p;
            if (layer.isToroidal()) {
                p = {static_cast<int>(px % layer.getWidth()), static_cast<int>(py % layer.getHeight())};
                p = layer.toToroidal(p);
            } else {
                if (px < 0 || py < 0 || px >= layer.getWidth() || py >= layer.getHeight()) continue;
                p = {static_cast<int>(px), static_cast<int>(py)};
            }

            if (dx == 0 && dy == 0 && !center) continue;

            if (layer.outOfBounds(p)) {
                if (layer.isToroidal()) {
                    p = layer.toToroidal(p);
                    std::invoke(std::forward<F>(f), p, result);
                }
            }
            else {
                std::invoke(std::forward<F>(f), p, result);
            }
        }
    }

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

