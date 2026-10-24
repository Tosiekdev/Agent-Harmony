# Experimental visualization

The optional visualization component uses SFML 3. It is separate from the core
library and its dependencies. Install SFML 3, then configure with:

```sh
cmake -S . -B build-visual -DAGENT_HARMONY_BUILD_VISUALIZATION=ON -DCMAKE_PREFIX_PATH=/path/to/sfml
cmake --build build-visual --config Release
```

SFML 3 builds static libraries by default, while `find_package(SFML)` selects
shared libraries by default. CI builds SFML with `-DBUILD_SHARED_LIBS=ON`.
On Linux, CI also sets `-DCMAKE_INSTALL_RPATH='$ORIGIN'` so SFML's shared libraries
can load their SFML dependencies from the same temporary installation directory.
For an existing static SFML installation, pass `-DSFML_STATIC_LIBRARIES=ON`
when configuring this library and any installed visualization consumers.

Installed consumers request `find_package(AgentHarmony CONFIG REQUIRED COMPONENTS
visualization)` and link `AgentHarmony::visualization`. Include
`<experimental/visualization/Visualization.hpp>` or
`<experimental/visualization/FieldPortrayal.hpp>`.

`FieldPortrayal` borrows a `Field` whose agents provide `getColor()` convertible to
`sf::Color`. `Visualization` borrows its portrayals; keep the model, agents, field,
and portrayals alive while the window runs. The installed headers are checked
independently and the visualization consumer compiles its portrayal template.
Opening a window requires a graphical session and is not part of headless CI.

This section documents the experimental component separately; the generated core
API reference does not require SFML.
