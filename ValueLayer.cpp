#include "ValueLayer.hpp"

namespace abmf {
template<typename T>
std::vector<T>& ValueLayer<T>::operator[](size_t index) {
    return grid[index];
}

template class ValueLayer<int>;
template class ValueLayer<double>;
}
