cmake_minimum_required(VERSION 3.29)
project(Agent-Harmony)

set(CMAKE_CXX_STANDARD 20)

include_directories(SFML/include)

add_library(Agent-Harmony STATIC src/experimental/visualization/Visualization.cpp
        src/experimental/visualization/utils/Control.cpp)
