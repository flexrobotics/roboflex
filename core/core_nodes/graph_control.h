#ifndef ROBOFLEX_GRAPH_CONTROL__H
#define ROBOFLEX_GRAPH_CONTROL__H

#include "core/node.h"
#include "frequency_generator.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * A node that can be placed at the root of a graph of nodes,
 * and that provides useful functions over the graph.
 * 
 * It can start() all RunnableNodes in the graph, and stop() them.
 * 
 * It provides several methods to walk nodes, walk connections,
 * and prune the graph.
 */
class GraphController: public RunnableNode {
public:

    GraphController(const string& name = "GraphController");

    void start() override;
    void stop() override;

    using NodeWalkCallback = std::function<void(NodePtr)>;
    void walk_nodes(NodeWalkCallback node_fun, bool forwards);
    void walk_nodes_forwards(NodeWalkCallback node_fun);
    void walk_nodes_backwards(NodeWalkCallback node_fun);

    using ConnectionWalkCallback = std::function<void(NodePtr, NodePtr)>;
    void walk_connections(ConnectionWalkCallback connection_fun, bool forwards);
    void walk_connections_forwards(ConnectionWalkCallback connection_fun);
    void walk_connections_backwards(ConnectionWalkCallback connection_fun);

    using NodeFilterCallback = std::function<bool(NodePtr)>;
    void filter_nodes(NodeFilterCallback filter_fun);
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_GRAPH_CONTROL__H
