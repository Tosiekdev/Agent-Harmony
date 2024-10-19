#include "ValueLayer.hpp"

#include <unistd.h>

namespace abmf {
template<typename T>
T ValueLayer<T>::get(Point pos) {
    return read[pos.y][pos.x];
}

template<typename T>
std::vector<Point> ValueLayer<T>::getMoore(Point pos, size_t radius) {
    auto [x, y] = pos;
    return {
        {x - 1, y + 1},
        {x, y + 1},
        {x + 1, y + 1},
        {x + 1, y},
        {x + 1, y - 1},
        {x, y - 1},
        {x - 1, y - 1},
        {x - 1, y},
        pos};
}

template<typename T>
std::vector<Point> ValueLayer<T>::getVonNeumann(Point pos, size_t radius) {
    auto [x, y] = pos;
    return {
        {x, y + 1},
        {x + 1, y},
        {x, y - 1},
        {x - 1, y},
        {pos}
};
}

template<typename T>
void ValueLayer<T>::swap() {
    read.swap(write);
}

template class ValueLayer<int>;
template class ValueLayer<double>;
}
