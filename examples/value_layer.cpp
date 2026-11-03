#include <space/ValueLayer.hpp>

int main() {
    agh::ValueLayer<int> values(4, 2, true, 0);
    values.set({7, 3}, 7); // Write buffer, wrapped coordinates.
    if (values.get({3, 1}) != 0 || values.getFromWrite({3, 1}) != 7) return 1;
    values.swap(); // Commit the complete write buffer as the read buffer.
    values.apply([](int& value) { value = 2; });
    return values.get({3, 1}) == 7 && values.getFromWrite({3, 1}) == 2 ? 0 : 1;
}
