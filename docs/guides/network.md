# Networks

`Network<Node, Label>` supports directed or undirected graphs, self-loops, optional
weights, and multiple connections between a pair when labels differ. Node and
label types need compatible hashing and equality. Adding an edge creates missing
nodes; adding an existing node returns its current iterator.

<!-- example: network.cpp -->

`removeEdges(node)` removes outgoing edges in directed graphs and incident edges
in undirected graphs. `deleteNode(node)` removes every incident edge and all indexes.
Missing endpoints in `removeEdge(from, to)` throw `std::out_of_range`; other
missing removals do nothing. Missing `getEdge` returns null; missing `getEdges` throws.

Mutable graphs expose mutable node lists and iterators. Preserve list structure
and equality/hash identity; use graph methods for insertion/deletion and `updateNode`
for lookup-based data updates. Edges borrowed from the graph become invalid when
erased; node deletion invalidates its iterator and all incident edge references.
Iterator-based edge removal requires live endpoints from the same graph.

Copying a network is disabled; use moving to transfer its nodes and relationships.
Rebuild nodes and edges explicitly when an independent graph copy is needed.
