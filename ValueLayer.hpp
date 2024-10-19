#pragma once

#include <vector>

#include "space/Neighbourhood.hpp"
#include "space/Point.hpp"

namespace abmf {
template<typename T>
class ValueLayer {
public:
    explicit ValueLayer(size_t width, size_t height) : read(height, std::vector<T>(width)), write(read) {}
    T get(Point pos);
    std::vector<Point> getMoore(Point pos, size_t radius=1);
    std::vector<Point> getVonNeumann(Point pos, size_t radius=1);
    void swap();

private:
    std::vector<std::vector<T>> read;
    std::vector<std::vector<T>> write;
};

using IntValueLayer = ValueLayer<int>;
using RealValueLayer = ValueLayer<double>;
}
