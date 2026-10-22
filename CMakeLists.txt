cmake_minimum_required(VERSION 3.29)
project(AgentHarmony VERSION 0.1.0 LANGUAGES CXX)

add_library(agent_harmony_core INTERFACE)
add_library(AgentHarmony::core ALIAS agent_harmony_core)
set_target_properties(agent_harmony_core PROPERTIES EXPORT_NAME core)
target_include_directories(agent_harmony_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(agent_harmony_core INTERFACE cxx_std_20)

option(AGENT_HARMONY_BUILD_VISUALIZATION "Build experimental SFML visualization" OFF)
if(AGENT_HARMONY_BUILD_VISUALIZATION)
    find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window System)
    add_library(agent_harmony_visualization STATIC
        src/experimental/visualization/Visualization.cpp
        src/experimental/visualization/utils/Control.cpp)
    add_library(AgentHarmony::visualization ALIAS agent_harmony_visualization)
    target_link_libraries(agent_harmony_visualization PUBLIC
        AgentHarmony::core SFML::Graphics SFML::Window SFML::System)
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
