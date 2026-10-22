# Networks

`Network<Node, Label>` supports directed or undirected graphs, self-loops, optional
weights, and multiple connections between a pair when labels differ. Node and
label types need compatible hashing and equality. Adding an edge creates missing
nodes; adding an existing node returns its current iterator.

<!-- example: network.cpp -->

`removeEdges(node)` removes outgoing edges in directed graphs and incident edges
in undirected graphs. `deleteNode(node)` removes every incident edge and all indexes.
Missing removals do nothing. Missing `getEdge` returns null; missing `getEdges`
throws `std::out_of_range`.

Nodes and returned iterators are read-only. Use `updateNode` for data updates that
preserve equality/hash identity. Edges borrowed from the graph become invalid when
erased; node deletion invalidates its iterator and all incident edge references.
Iterator-based edge removal requires live endpoints from the same graph.

Copying a network is disabled; use moving to transfer its nodes and relationships.
Rebuild nodes and edges explicitly when an independent graph copy is needed.
