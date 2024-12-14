cmake_minimum_required(VERSION 3.29)
project(ABMframework)

set(CMAKE_CXX_STANDARD 20)

add_library(ABMframework STATIC Model.cpp
        include/model/ModelImpl.hpp
        include/schedule/Schedule.hpp
        include/schedule/ScheduleImpl.hpp
        include/space/ValueLayer.hpp
        include/space/ValueLayerImpl.hpp
        include/space/Point.hpp
        include/space/Field.hpp
        include/space/FieldImpl.hpp
        include/utilities/Utils.hpp
        include/space/MultiagentField.hpp
        include/space/MultiagentFieldImpl.hpp
        include/utilities/Concepts.hpp
        include/space/Edge.hpp
        include/space/Network.hpp
        include/space/NetworkImpl.hpp
        include/space/Space.hpp
        include/AgentHarmony.hpp
)
