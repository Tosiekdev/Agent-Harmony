#pragma once

#include <vector>

#include "space/Neighbourhood.hpp"
#include "space/Point.hpp"

namespace abmf {
template<typename T>
class ValueLayer {
public:
    explicit ValueLayer(size_t width, size_t height) : grid(height, std::vector<T>(width)) {}
    T& get(Point pos);
    Moore<T> getMoore(Point pos);
    VonNeumann<T> getVonNeumann(Point pos);
    T sumMoore(bool center);

private:
    std::vector<std::vector<T>> grid;
};

using IntValueLayer = ValueLayer<int>;
using RealValueLayer = ValueLayer<double>;
}
