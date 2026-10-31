cmake_minimum_required(VERSION 3.29)
project(AgentHarmony VERSION 0.1.0 LANGUAGES CXX)

option(AGENT_HARMONY_BUILD_DOCS "Create a core translation unit for MrDocs" OFF)
if(MY_PROJECT_MRDOCS_BUILD OR AGENT_HARMONY_BUILD_DOCS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

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

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(TARGETS agent_harmony_core EXPORT AgentHarmonyTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "experimental" EXCLUDE)
install(EXPORT AgentHarmonyTargets NAMESPACE AgentHarmony::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AgentHarmony)
if(AGENT_HARMONY_BUILD_VISUALIZATION)
    set_target_properties(agent_harmony_visualization PROPERTIES EXPORT_NAME visualization)
    install(TARGETS agent_harmony_visualization EXPORT AgentHarmonyVisualizationTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/experimental/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/experimental)
    install(EXPORT AgentHarmonyVisualizationTargets NAMESPACE AgentHarmony::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AgentHarmony)
endif()
configure_package_config_file(cmake/AgentHarmonyConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AgentHarmonyConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AgentHarmony)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/AgentHarmonyConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/AgentHarmonyConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AgentHarmonyConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AgentHarmony)

if(MY_PROJECT_MRDOCS_BUILD OR AGENT_HARMONY_BUILD_DOCS)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/all_headers.cpp "#include <AgentHarmony.hpp>\n")
    add_library(agent_harmony_documentation OBJECT ${CMAKE_CURRENT_BINARY_DIR}/all_headers.cpp)
    target_link_libraries(agent_harmony_documentation PRIVATE AgentHarmony::core)
endif()

option(AGENT_HARMONY_BUILD_EXAMPLES "Build the executable documentation examples" OFF)
if(AGENT_HARMONY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
