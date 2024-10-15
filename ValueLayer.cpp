#include "ValueLayer.hpp"

namespace abmf {
template ValueLayer<int>;
template ValueLayer<double>;

template<typename T>
std::vector<T>& ValueLayer<T>::operator[](size_t index) {
    return grid[index];
}
}
