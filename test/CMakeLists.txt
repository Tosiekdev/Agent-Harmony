cmake_minimum_required(VERSION 3.29)
project(ABMframeworkTest)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(..)

add_executable(
        ABMframeworkTest
        ModelTest.cpp
        ScheduleTest.cpp
        ValueLayerTest.cpp
        FieldTest.cpp
        MultiagentFieldTest.cpp
        NetworkTest.cpp
        ContinuousSpaceTest.cpp
)
target_link_libraries(
        ABMframeworkTest
        GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(ABMframeworkTest)
