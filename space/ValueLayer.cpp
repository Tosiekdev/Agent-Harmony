#include "ValueLayer.hpp"

namespace abmf {
namespace {
template<typename T>
auto visitNeighborhood(const ValueLayer<T>& layer, const Point pos, const int r, const bool moore,
                       const bool center, auto f) -> std::vector<std::result_of_t<decltype(f)(Point)>> {
    std::vector<std::result_of_t<decltype(f)(Point)>> result;
    if (moore) {
        result.reserve((2 * r + 1) * (2 * r + 1));
    }
    else {
        result.reserve(r * r + (r + 1) * (r + 1));
    }
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (!moore && dx + dy > r) continue;

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
}

template<typename T>
T ValueLayer<T>::get(Point pos) {
    return read[pos.y][pos.x];
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
Point ValueLayer<T>::toToroidal(Point p) const {
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
void ValueLayer<T>::swap() {
    read.swap(write);
}

template class ValueLayer<int>;
template class ValueLayer<double>;
}
