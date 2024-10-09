#include <gtest/gtest.h>

#include "../Model.hpp"
#include "../Agent.hpp"

struct MyAgent : abmf::Agent {
    template<typename T>
    void step() {
        std::cout << T();
    }
};

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    abmf::Model<int, double> m;
    MyAgent a;
    a.step<int>();
    ASSERT_EQ(m.getAgents<int>().size(), 0);
}
