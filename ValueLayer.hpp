#pragma once

#include <vector>

namespace abmf {
template<typename T>
class ValueLayer {
public:
    explicit ValueLayer(size_t width, size_t height) : grid(std::vector<T>(width), height) {}
    std::vector<T>& operator[](size_t index);

private:
    std::vector<std::vector<T>> grid;
};

using IntValueLayer = ValueLayer<int>;
using RealValueLayer = ValueLayer<double>;

}
