cmake_minimum_required(VERSION 3.29)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(AgentHarmonyTests LANGUAGES CXX)
    enable_testing()
    add_library(agent_harmony_core INTERFACE)
    add_library(AgentHarmony::core ALIAS agent_harmony_core)
    target_include_directories(agent_harmony_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_compile_features(agent_harmony_core INTERFACE cxx_std_20)
endif()

include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(ABMframeworkTest
    ModelTest.cpp ScheduleTest.cpp ValueLayerTest.cpp FieldTest.cpp
    MultiagentFieldTest.cpp NetworkTest.cpp NetworkSafetyTest.cpp ContinuousSpaceTest.cpp SpaceSafetyTest.cpp)
target_link_libraries(ABMframeworkTest PRIVATE AgentHarmony::core GTest::gtest_main)
option(AGENT_HARMONY_SANITIZERS "Enable address and undefined-behavior sanitizers in tests" OFF)
if(AGENT_HARMONY_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Sanitizer tests require Clang or GCC")
    endif()
    target_compile_options(ABMframeworkTest PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(ABMframeworkTest PRIVATE -fsanitize=address,undefined)
endif()
include(GoogleTest)
gtest_discover_tests(ABMframeworkTest)
add_test(NAME AllTestsInOneProcess COMMAND ABMframeworkTest)
