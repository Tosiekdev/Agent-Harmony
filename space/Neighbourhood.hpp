#pragma once
#include <stdexcept>

namespace abmf {
template<class T>
struct Moore {
    T &first, &second, &third, &fourth, &fifth, &sixth, &seventh, &eighth, &center;

    T& operator[](const size_t i) {
        switch (i) {
        case 0:
            return first;
        case 1:
            return second;
        case 2:
            return third;
        case 3:
            return fourth;
        case 4:
            return fifth;
        case 5:
            return sixth;
        case 6:
            return seventh;
        case 7:
            return eighth;
        case 8:
            return center;
        default:
            throw std::invalid_argument("Moore's neighbourhood has only 9 elements (including center)!");
        }
    }
};

template<class T>
struct VonNeumann {
    T &first, &second, &third, &fourth, &center;

    T& operator[](const size_t i) {
        switch (i) {
        case 0:
            return first;
        case 1:
            return second;
        case 2:
            return third;
        case 3:
            return fourth;
        case 4:
            return center;
        default:
            throw std::invalid_argument("VonNeumann's neighbourhood has only 5 elements (including center)!");
        }
    }
};
}
