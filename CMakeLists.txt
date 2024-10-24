cmake_minimum_required(VERSION 3.29)
project(ABMframework)

set(CMAKE_CXX_STANDARD 20)

add_library(ABMframework STATIC Model.cpp
        ModelImpl.hpp
        AgentBase.hpp
        Schedule.hpp
        ScheduleImpl.hpp
        space/ValueLayer.hpp
        space/ValueLayer.cpp
        space/Point.hpp
)
