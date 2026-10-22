#pragma once

#include "Concepts.hpp"
#include "Validation.hpp"
#include "../space/Point.hpp"

#include <cmath>
#include <concepts>
#include <functional>
#include <vector>

namespace agh {
inline Point convertToToroidal(const Point p, const int width, const int height) {
    return detail::checkedPosition(p, width, height, true);

}

template<typename T>
concept Grid = requires(T t) {
    { t.outOfBounds(Point{}) } -> std::same_as<bool>;
    { t.isToroidal() } -> std::same_as<bool>;
    { t.toToroidal(Point{}) } -> std::same_as<Point>;
};

namespace detail {
template<Grid T, std::invocable<Point> F>
void forEachNeighborhoodPoint(const T& layer, Point pos, int radius, bool moore, bool center, F&& f) {
    if (radius < 0) throw std::invalid_argument("Radius must be nonnegative");
    pos = checkedPosition(pos, layer.getWidth(), layer.getHeight(), layer.isToroidal());
    const long long r = radius;
    const long long minX = layer.isToroidal() ? 0 : std::max(0LL, pos.x - r);
    const long long minY = layer.isToroidal() ? 0 : std::max(0LL, pos.y - r);
    const long long maxX = layer.isToroidal() ? layer.getWidth() - 1 : std::min<long long>(layer.getWidth() - 1, pos.x + r);
    const long long maxY = layer.isToroidal() ? layer.getHeight() - 1 : std::min<long long>(layer.getHeight() - 1, pos.y + r);
    for (long long y = minY; y <= maxY; ++y) {
        for (long long x = minX; x <= maxX; ++x) {
            Point point{static_cast<int>(x), static_cast<int>(y)};
            if (!center && point == pos) continue;
            long long dx = std::abs(x - pos.x), dy = std::abs(y - pos.y);
            if (layer.isToroidal()) {
                dx = std::min(dx, layer.getWidth() - dx);
                dy = std::min(dy, layer.getHeight() - dy);
            }
            if ((moore ? std::max(dx, dy) : dx + dy) <= r) std::invoke(f, point);
        }
    }
}
}

template<Grid T, std::invocable<Point> F>
auto visitNeighborhood(const T& layer, Point pos, int r, bool moore,
                       bool center, F&& f) -> std::vector<std::invoke_result_t<F, Point>> {
    std::vector<std::invoke_result_t<F, Point>> result;
    detail::forEachNeighborhoodPoint(layer, pos, r, moore, center,
        [&](Point point) { result.push_back(std::invoke(f, point)); });
    return result;
}

template<Grid T, typename A, std::invocable<Point, std::vector<A>&> F>
auto visitNeighbors(const T& layer, Point pos, int r, bool moore,
                    bool center, F&& f) -> std::vector<A> {
    std::vector<A> result;
    detail::forEachNeighborhoodPoint(layer, pos, r, moore, center,
        [&](Point point) { std::invoke(f, point, result); });
    return result;
}

template<typename T, std::invocable<T&> F>
void applyToAll(std::vector<T>& grid, F&& f, const int width, const int height) {
    if (grid.empty()) return;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), grid[static_cast<std::size_t>(y) * width + x]);
        }
    }
}

template<typename T, std::invocable<Point, T&> F>
void transformAll(std::vector<T>& grid, F&& f, const int width, const int height) {
    if (grid.empty()) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), grid[static_cast<std::size_t>(y) * width + x]);
        }
    }
}

inline float l2(const RealPoint p1, const RealPoint p2) {
    return std::hypot(p2.x - p1.x, p2.y - p1.y);
}

inline auto Pos = [](auto a) {
    return *a->pos;
};
}

