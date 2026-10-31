# Value layers

`ValueLayer<T>` separates values read during an update from values being written.
`get` reads the read buffer; `set` and `getFromWrite` access the write buffer.
`swap()` exchanges the complete buffers without copying their elements.

<!-- example: value_layer.cpp -->

`setOnRead` updates the read buffer explicitly. `apply` and `transform` update the
write buffer. After swapping, the former read buffer becomes the new write buffer;
it is not automatically reset or copied from the current read buffer.

Neighborhood queries return read-buffer values. Coordinate validation and torus
normalization match the [discrete agent spaces](spaces.md). Standalone value
layers retain ordinary value-copy semantics.
