# Build and install

Requirements: CMake 3.29 or later and a C++20 compiler. CI covers GCC and Clang on
Linux, Apple Clang on macOS, and MSVC on Windows. Core requires neither SFML nor
Python/documentation tools.

The main unit suite runs on every platform. Linux additionally checks isolated
headers, installed consumers, shuffled execution, guide examples, sanitizers,
optional visualization, and the generated documentation. macOS and Windows CI
build and run only the main unit-suite target.

## Configure, test, and install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DAGENT_HARMONY_BUILD_EXAMPLES=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
cmake --install build --prefix ./install --config Release
```

Tests fetch GoogleTest at pinned revision
`03597a01ee50ed33e9dfd640b249b4be3799d395`. For offline operation, add
`-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/to/that/revision`.
For an installation without tests, use `-DBUILD_TESTING=OFF`.

## Use the installed package

```cmake
cmake_minimum_required(VERSION 3.29)
project(MySimulation LANGUAGES CXX)
find_package(AgentHarmony CONFIG REQUIRED)
add_executable(simulation main.cpp)
target_link_libraries(simulation PRIVATE AgentHarmony::core)
```

Configure the consumer with `-DCMAKE_PREFIX_PATH=/absolute/path/to/install`.
The exported target supplies include paths and the C++20 feature requirement.
For an embedded checkout, use `add_subdirectory(path/to/Agent-Harmony)` and link
the same `AgentHarmony::core` target.

## Options

| Option | Default | Purpose |
| --- | --- | --- |
| `BUILD_TESTING` | `ON` | GoogleTest, whole-process/shuffled runs, header and installed-consumer checks |
| `AGENT_HARMONY_SANITIZERS` | `OFF` | ASan/UBSan instrumentation for GCC/Clang tests |
| `AGENT_HARMONY_BUILD_EXAMPLES` | `OFF` | Build the executable guide examples |
| `AGENT_HARMONY_BUILD_VISUALIZATION` | `OFF` | Build the optional SFML 3 component |
| `AGENT_HARMONY_BUILD_DOCS` | `OFF` | Generate the compile database and core extraction target for MrDocs |

Documentation dependencies are optional; see [documentation development](documentation.md).
