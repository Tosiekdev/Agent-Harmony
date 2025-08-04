if (MY_PROJECT_MRDOCS_BUILD)
    cmake_minimum_required(VERSION 3.29)
    project(my_project_mrdocs_target)
    # Glob all header files
    set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
    file(GLOB_RECURSE HEADER_FILES "${INCLUDE_DIR}/**/*.hpp")

    # Create a temporary source file that includes all header files
    set(TEMP_CPP_FILE "${CMAKE_CURRENT_BINARY_DIR}/all_headers.cpp")
    file(WRITE ${TEMP_CPP_FILE} "// This file is generated automatically by CMake\n\n")
    foreach(HEADER_FILE ${HEADER_FILES})
        file(APPEND ${TEMP_CPP_FILE} "#include \"${HEADER_FILE}\"\n")
    endforeach()
    include_directories(SFML/include)

    # Create a custom target for MrDocs
    add_library(my_project_mrdocs_target ${TEMP_CPP_FILE})

    # Set any other target properties here
    target_include_directories(my_project_mrdocs_target PRIVATE ${INCLUDE_DIR})
    target_link_libraries(my_project_mrdocs_target PRIVATE sfml-graphics sfml-window sfml-system Agent-Harmony)

    # Don't create any other targets
    return()
else ()
    cmake_minimum_required(VERSION 3.29)
    project(Agent-Harmony)

    set(CMAKE_CXX_STANDARD 20)

    include_directories(SFML/include)

    add_library(Agent-Harmony STATIC src/experimental/visualization/Visualization.cpp
            src/experimental/visualization/utils/Control.cpp)
endif()
