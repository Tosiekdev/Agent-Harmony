#pragma once

#include "../Utils.hpp"
#include "ValueLayer.hpp"

#include <functional>

namespace abmf {
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
void ValueLayer<T>::setOnRead(Point pos, T value) {
    read[pos.y][pos.x] = value;
}

template<typename T>
template<std::invocable<T&> F>
void ValueLayer<T>::apply(F&& f) {
    applyToAll(write, std::forward<F>(f));
}

template<typename T>
template<std::invocable<Point, T> F>
void ValueLayer<T>::forEach(F&& f) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::invoke(std::forward<F>(f), Point(x, y), read[y][x]);
        }
    }
}

template<typename T>
template<std::invocable<Point, T&> F>
void ValueLayer<T>::transform(F&& f) {
    transformAll(write, std::forward<F>(f));
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
    return p.x < 0 || p.x >= width || p.y < 0 || p.y >= height;
}

template<typename T>
Point ValueLayer<T>::toToroidal(const Point p) const {
    return convertToToroidal(p, width, height);
}

template<typename T>
void ValueLayer<T>::swap() {
    read.swap(write);
}
}
