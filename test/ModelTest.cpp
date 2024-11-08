#include <gtest/gtest.h>

#include <optional>

#include "../Model.hpp"

namespace test::model {
struct MyAgent {
    explicit MyAgent(const int pId) : id(pId) {}
    int id;
    bool isActive() {
        return true;
    }
};

struct MyModel : abmf::Model<MyAgent> {};

TEST(ModelTest, AddingAgent) {
    MyModel m;
    m.emplaceAgent<MyAgent>(1);
    ASSERT_EQ(m.agentCount(), 1);
    EXPECT_EQ(m.getAgents<MyAgent>()[0].id, 1);
}
}
