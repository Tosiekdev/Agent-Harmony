#pragma once

#include <algorithm>

#include "../utilities/Utils.hpp"

#include <functional>

namespace agh {
template<typename T>
T ValueLayer<T>::get(const Point pos) const {
    return read[pos.y * width + pos.x];
}

template<typename T>
T ValueLayer<T>::getFromWrite(const Point pos) const {
    return write[pos.y * width + pos.x];
}

template<typename T>
void ValueLayer<T>::set(const Point pos, T value) {
    write[pos.y * width + pos.x] = value;
}

template<typename T>
void ValueLayer<T>::setOnRead(const Point pos, T value) {
    read[pos.y * width + pos.x] = value;
}

template<typename T>
template<std::invocable<T&> F>
void ValueLayer<T>::apply(F&& f) {
    std::for_each(write.begin(), write.end(), f);
}

template<typename T>
template<std::invocable<Point, T&> F>
void ValueLayer<T>::transform(F&& f) {
    transformAll(write, std::forward<F>(f), width, height);
}

template<typename T>
std::vector<Point> ValueLayer<T>::getNeighborhood(
    const Point pos, const int r, const bool moore, const bool center) const {
    return visitNeighborhood(*this, pos, r, moore, center, [](Point p) { return p; });
}

template<typename T>
std::vector<T> ValueLayer<T>::getNeighbors(const Point pos, const int r, const bool moore, const bool center) const {
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
