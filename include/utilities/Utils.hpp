#pragma once

#include "include/space/Point.hpp"

#include <cmath>
#include <concepts>
#include <functional>
#include <vector>

namespace abmf {
inline Point convertToToroidal(const Point p, const int width, const int height) {
    if (p.x < 0) {
        if (p.y < 0) {
            return {width + p.x % width, height + p.y % height};
        }
        return {width + p.x % width, p.y % height};
    }
    if (p.y < 0) {
        return {p.x, height + p.y % height};
    }
    return {p.x % width, p.y % height};
}

template<typename T>
concept Grid = requires(T t) {
    { t.outOfBounds(Point{}) } -> std::same_as<bool>;
    { t.isToroidal() } -> std::same_as<bool>;
    { t.toToroidal(Point{}) } -> std::same_as<Point>;
};

template<Grid T, std::invocable<Point> F>
auto visitNeighborhood(const T& layer, const Point pos, const int r, const bool moore,
                       const bool center, F&& f) -> std::vector<std::invoke_result_t<F, Point>> {
    std::vector<std::invoke_result_t<F, Point>> result;
    if (moore) {
        result.reserve((2 * r + 1) * (2 * r + 1));
    }
    else {
        result.reserve(r * r + (r + 1) * (r + 1));
    }
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (!moore && std::abs(dx) + std::abs(dy) > r) continue;

            Point p = {pos.x + dx, pos.y + dy};

            if (p == pos && !center) continue;

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

template<typename T, std::invocable<T&> F>
void applyToAll(std::vector<std::vector<T>>& grid, F&& f) {
    if (grid.empty()) return;
    for (size_t i = 0; i < grid.size(); ++i) {
        for (size_t j = 0; j < grid[0].size(); ++j) {
            std::invoke(std::forward<F>(f), grid[i][j]);
        }
    }
}

template<typename T, std::invocable<T&> F>
void applyToAll(std::vector<T>& grid, F&& f, const int width, const int height) {
    if (grid.empty()) return;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), grid[y * width + x]);
        }
    }
}

template<typename T, std::invocable<Point, T&> F>
void transformAll(std::vector<std::vector<T>>& grid, F&& f) {
    if (grid.empty()) return;
    for (int y = 0; y < grid.size(); ++y) {
        for (int x = 0; x < grid[0].size(); ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), grid[y][x]);
        }
    }
}

template<typename T, std::invocable<Point, T&> F>
void transformAll(std::vector<T>& grid, F&& f, const int width, const int height) {
    if (grid.empty()) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), grid[y * width + x]);
        }
    }
}
}

