#include "graph_control.h"

namespace roboflex {
namespace nodes {

using std::set;

// -- some utility functions --

void step_nodes(GraphController::NodeWalkCallback node_fun, NodePtr node, set<NodePtr>& visited, bool forwards)
{
    if (visited.find(node) != visited.end()) {
        return;
    }

    visited.insert(node);

    auto children = node->get_observers();
    if (forwards) {
        node_fun(node);
        for (auto& child: children) {
            step_nodes(node_fun, child, visited, true);
        }
    } else {
        for (auto& child: children) {
            step_nodes(node_fun, child, visited, false);
        }
        node_fun(node);
    }
}

void step_connections(GraphController::ConnectionWalkCallback connection_fun, NodePtr node, set<NodePtr>& visited, bool forwards)
{
    if (visited.find(node) != visited.end()) {
        return;
    }

    visited.insert(node);

    auto children = node->get_observers();
    for (auto& child: children) {
        if (forwards) {
            connection_fun(node, child);
            step_connections(connection_fun, child, visited, true);
        } else {
            step_connections(connection_fun, child, visited, false);
            connection_fun(node, child);
        }
    }
}


// -- GraphController --

GraphController::GraphController(const string& name):
    RunnableNode(name)
{

}

void GraphController::start()
{
    this->walk_nodes_backwards([](NodePtr node){
        auto rn = std::dynamic_pointer_cast<RunnableNode>(node);
        if (rn) {
            rn->start();
        }
    });
}

void GraphController::stop()
{
    this->walk_nodes_forwards([](NodePtr node){
        auto rn = std::dynamic_pointer_cast<RunnableNode>(node);
        if (rn) {
            rn->stop();
        }
    });
}

void GraphController::walk_nodes(NodeWalkCallback node_fun, bool forwards)
{
    set<NodePtr> visited;
    auto children = this->get_observers();
    for (auto& child: children) {
        step_nodes(node_fun, child, visited, forwards);
    }
}

void GraphController::walk_nodes_forwards(NodeWalkCallback node_fun)
{
    walk_nodes(node_fun, true);
}

void GraphController::walk_nodes_backwards(NodeWalkCallback node_fun)
{
    walk_nodes(node_fun, false);
}

void GraphController::walk_connections(ConnectionWalkCallback connection_fun, bool forwards)
{
    set<NodePtr> visited;
    auto children = this->get_observers();
    for (auto& child: children) {
        step_connections(connection_fun, child, visited, forwards);
    }
}

void GraphController::walk_connections_forwards(ConnectionWalkCallback connection_fun)
{
    walk_connections(connection_fun, true);
}
    
void GraphController::walk_connections_backwards(ConnectionWalkCallback connection_fun)
{
    walk_connections(connection_fun, false);
}

void filter(GraphController::NodeFilterCallback filter_fun, Node* parent, NodePtr child, set<NodePtr>& visited)
{
    if (visited.find(child) != visited.end()) {
        return;
    }

    visited.insert(child);

    bool keep_node = filter_fun(child);
    auto grand_children = child->get_observers();

    if (!keep_node) {
        parent->disconnect(child);
        for (auto& grand_child: grand_children) {
            parent->connect(grand_child);
        }
    } 

    for (auto& grand_child: grand_children) {
        filter(filter_fun, child.get(), grand_child, visited);
    }
}

void GraphController::filter_nodes(NodeFilterCallback filter_fun)
{
    set<NodePtr> visited;
    auto children = this->get_observers();
    for (auto& child: children) {
        filter(filter_fun, this, child, visited);
    }
}

} // namespace nodes
} // namespace roboflex
