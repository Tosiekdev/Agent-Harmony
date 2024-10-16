#include "ValueLayer.hpp"

namespace abmf {
template<typename T>
T& ValueLayer<T>::get(Point pos) {
    return grid[pos.y][pos.x];
}

template<typename T>
Moore<T> ValueLayer<T>::getMoore(Point pos) {
    auto [x, y] = pos;
    return Moore<T>(
        get({x - 1, y + 1}),
        get({x, y + 1}),
        get({x + 1, y + 1}),
        get({x + 1, y}),
        get({x + 1, y - 1}),
        get({x, y - 1}),
        get({x - 1, y - 1}),
        get({x - 1, y}),
        get(pos));
}

template<typename T>
VonNeumann<T> ValueLayer<T>::getVonNeumann(Point pos) {
    auto [x, y] = pos;
    return VonNeumann<T>(
        get({x, y + 1}),
        get({x + 1, y}),
        get({x, y - 1}),
        get({x - 1, y}),
        get(pos));
}

template class ValueLayer<int>;
template class ValueLayer<double>;
}
