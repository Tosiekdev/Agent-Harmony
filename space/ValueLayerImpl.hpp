#pragma once

#include "../Utils.hpp"
#include "ValueLayer.hpp"

#include <cmath>
#include <functional>
#include <type_traits>

namespace abmf {

template<typename T>
auto visitNeighborhood(const ValueLayer<T>& layer, const Point pos, const int r, const bool moore,
                       const bool center, auto f) -> std::vector<std::invoke_result_t<decltype(f), Point>> {
    std::vector<std::invoke_result_t<decltype(f), Point>> result;
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
                    result.push_back(f(p));
                }
            }
            else {
                result.push_back(f(p));
            }
        }
    }

    return result;
}

template<typename T>
T ValueLayer<T>::get(Point pos) {
    return read[pos.y][pos.x];
}

template<typename T>
T ValueLayer<T>::getFromWrite(Point pos) {
    return write[pos.y][pos.x];
}

template<typename T>
void ValueLayer<T>::set(Point pos, T value) {
    write[pos.y][pos.x] = value;
}

template<typename T>
template<std::invocable<T&> F>
void ValueLayer<T>::apply(F&& f) {
    for (auto& row : read) {
        for (auto& val : row) {
            std::invoke(std::forward<F>(f), val);
        }
    }
}

template<typename T>
template<std::invocable<Point, T> F>
void ValueLayer<T>::forEach(F&& f) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), write[y][x]);
        }
    }
}

template<typename T>
template<std::invocable<Point, T&> F>
void ValueLayer<T>::transform(F&& f) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), write[y][x]);
        }
    }
}

template<typename T>
std::vector<Point> ValueLayer<T>::getNeighborhood(
    const Point pos, const int r, const bool moore, const bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<typename T>
std::vector<T> ValueLayer<T>::getNeighbors(const Point pos, const int r, const bool moore, const bool center) {
    return visitNeighborhood(*this, pos, r, moore, center, [&](const Point p) { return get(p); });
}

template<typename T>
bool ValueLayer<T>::outOfBounds(const Point p) const {
    return p.x < 0 || p.x > width || p.y < 0 || p.y > height;
}

template<typename T>
Point ValueLayer<T>::toToroidal(const Point p) const {
    return toToroidal(p, width, height);
}

template<typename T>
void ValueLayer<T>::swap() {
    read.swap(write);
}

}
