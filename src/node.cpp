#include <sstream>
#include <signal.h>
#include "roboflex_core/node.h"
#include "roboflex_core/util/utils.h"
#include "roboflex_core/core_messages/core_messages.h"

namespace roboflex::core {

Node::Node(const std::string& name):
    name(name),
    guid(sole::uuid4())
{

}

Node::~Node()
{
    
}

string Node::to_string() const
{
    std::stringstream sst;
    sst << "<Node"
        << " name: \"" << get_name() << "\""
        << " guid: " << get_guid();
    if (has_observers()) {
        sst << " children(" << observers.size() << "): [";
        for (auto n: observers) {
            sst << " \"" << n->get_name() << "\"";
        }
        sst << "]";
    }
    sst << ">";
    return sst.str();
}

string Node::graph_to_string(int level) const
{
    int init_level = level;
    std::stringstream sst;
    this->walk_nodes_forwards([init_level, &sst](NodePtr node, int level) {
        sst << repeated_string("  ", init_level+level)  << node->to_string() << std::endl;
    });
    return sst.str();
}


// --- Connection management ---

Node::NodePtr Node::connect(Node::NodePtr node)
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);
    observers.push_back(node);
    node->on_connect(*this, false);
    this->on_connect(*node, true);
    return node;
}

Node& Node::connect(Node &node)
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);

    // Creates a non-destructing smart pointer. This method is intended
    // for use by c++ programs that create nodes on the stack, and then
    // connects them, as opposed to python programs (or c++, or other)
    // that want to create a node, connect it, and then forget it.
    auto sptr = Node::NodePtr(&node, [](Node *) {});
    observers.push_back(sptr);
    node.on_connect(*this, false);
    this->on_connect(node, true);
    return node;
}

void Node::disconnect(Node::NodePtr node)
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);
    observers.remove(node);
}

void Node::disconnect(Node &node)
{
    auto sptr = Node::NodePtr(&node, [](Node *) {});
    disconnect(sptr);
}

bool Node::has_observers() const
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);
    return !observers.empty();
}

size_t Node::num_observers() const
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);
    return observers.size();
}

std::list<Node::NodePtr> Node::get_observers() const
{
    return observers;
}

Node& Node::operator > (Node& other) 
{
    return this->connect(other);
}

Node::NodePtr Node::operator > (Node::NodePtr other) 
{
    return this->connect(other);
}


// -- some utility functions --

/**
 * Walks the graph of nodes, starting at node, and calling node_fun on each node.
 */
void step_nodes(Node::NodeWalkCallback node_fun, NodePtr node, set<NodePtr>& visited, bool forwards, int level=0)
{
    if (visited.find(node) != visited.end()) {
        return;
    }

    visited.insert(node);

    auto children = node->get_observers();
    if (forwards) {
        node_fun(node, level);
        for (auto& child: children) {
            step_nodes(node_fun, child, visited, true, level+1);
        }
    } else {
        for (auto& child: children) {
            step_nodes(node_fun, child, visited, false, level+1);
        }
        node_fun(node, level);
    }
}

/**
 * Walks the graph of connections, starting at node, and calling connection_fun 
 * on each pair of connected nodes connection.
 */
void step_connections(Node::ConnectionWalkCallback connection_fun, NodePtr node, set<NodePtr>& visited, bool forwards, int level=0)
{
    if (visited.find(node) != visited.end()) {
        return;
    }

    visited.insert(node);

    auto children = node->get_observers();
    for (auto& child: children) {
        if (forwards) {
            connection_fun(node, child, level);
            step_connections(connection_fun, child, visited, true, level+1);
        } else {
            step_connections(connection_fun, child, visited, false, level+1);
            connection_fun(node, child, level);
        }
    }
}

/**
 * Prunes the graph of nodes, starting at node, and calling filter_fun 
 * on each node to determine whether to keep that node. The graph is pruned.
 */
void filter(Node::NodeFilterCallback filter_fun, Node* parent, NodePtr child, set<NodePtr>& visited, int level=0)
{
    if (visited.find(child) != visited.end()) {
        return;
    }

    visited.insert(child);

    bool keep_node = filter_fun(child, level);
    auto grand_children = child->get_observers();

    if (!keep_node) {
        parent->disconnect(child);
        for (auto& grand_child: grand_children) {
            parent->connect(grand_child);
        }
    } 

    for (auto& grand_child: grand_children) {
        filter(filter_fun, child.get(), grand_child, visited, level+1);
    }
}

void Node::walk_nodes(NodeWalkCallback node_fun, bool forwards) const
{
    set<NodePtr> visited;
    auto children = this->get_observers();
    for (auto& child: children) {
        step_nodes(node_fun, child, visited, forwards);
    }
}

void Node::walk_nodes_forwards(NodeWalkCallback node_fun) const
{
    walk_nodes(node_fun, true);
}

void Node::walk_nodes_backwards(NodeWalkCallback node_fun) const
{
    walk_nodes(node_fun, false);
}

void Node::walk_connections(ConnectionWalkCallback connection_fun, bool forwards) const
{
    set<NodePtr> visited;
    auto children = this->get_observers();
    for (auto& child: children) {
        step_connections(connection_fun, child, visited, forwards);
    }
}

void Node::walk_connections_forwards(ConnectionWalkCallback connection_fun) const
{
    walk_connections(connection_fun, true);
}
    
void Node::walk_connections_backwards(ConnectionWalkCallback connection_fun) const
{
    walk_connections(connection_fun, false);
}

void Node::filter_nodes(NodeFilterCallback filter_fun)
{
    set<NodePtr> visited;
    auto children = this->get_observers();
    for (auto& child: children) {
        filter(filter_fun, this, child, visited);
    }
}


// --- Signal and receive ---

void Node::notify_observers(MessagePtr m)
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);

    message_send_counter += 1;

    for (auto o: observers) {
        o->receive_from(m, *this);
    }
}

void Node::notify_self(MessagePtr m)
{
    {
        const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);
        message_send_counter += 1;
    }

    this->receive_from(m, *this);
}

MessagePtr Node::signal(MessagePtr m)
{
    m->set_sender_info(get_name(), get_guid(), message_send_counter);
    notify_observers(m);
    return m;
}

MessagePtr Node::signal_self(MessagePtr m)
{
    m->set_sender_info(get_name(), get_guid(), message_send_counter);
    notify_self(m);
    return m;
}

void Node::receive_from(MessagePtr m, const Node& /*from*/)
{
    receive(m);
}

void Node::receive(MessagePtr m)
{
    signal(m);
}

MessagePtr Node::handle_rpc(MessagePtr rpc_message)
{
    if (rpc_message->module_name() == CoreModuleName) {

        const auto message_name = rpc_message->message_name();

        if (message_name == PingMessageName) {
            return make_shared<BlankMessage>(PongMessageName);
        }
        else if (message_name == GetNameMessageName) {
            return make_shared<StringMessage>(GotNameMessageName, get_name());
        }
        else if (message_name == GetGuidMessageName) {
            return make_shared<StringMessage>(GotGuidMessageName, get_guid().str());
        }
    }

    return nullptr;
}


// --- RunnableNode ---

RunnableNode::RunnableNode(const std::string& name):
    Node(name)
{

}

RunnableNode::~RunnableNode()
{
    this->stop();
}

void RunnableNode::start()
{
    // Once we get jthreads in clang:
    // if (this->my_thread == nullptr) {
    //     auto f = [](std::stop_token stoken, RunnableNode* self) {
    //         self->stop_token = stoken;
    //         self->child_thread_fn();
    //     };
    //     this->my_thread.reset(new std::jthread(f, this));
    // }
    if (this->my_thread == nullptr) {
        auto f = [](RunnableNode* self) {
            self->child_thread_fn();
        };
        this->stop_signal = false;
        this->my_thread.reset(new std::thread(f, this));
    }
}

void RunnableNode::stop_and_join()
{
    if (this->my_thread != nullptr) {
        //this->my_thread->request_stop();
        this->request_stop();
        this->my_thread->join();
        this->my_thread.reset();
    }
}

void RunnableNode::request_stop()
{
    //if (this->my_thread != nullptr) {
        //this->my_thread->request_stop();
        this->stop_signal = true;
    //}
}

// We switch the signal handler if run() is called. When run()
// is called, we simply run start_thread_fn() inside the current
// thread, without launching another thread - we assume we are 
// in the main thread of the process.
std::atomic<RunnableNode*> running_node = NULL;

void interrupt_signal_handler(int /*sig*/)
{
    RunnableNode *rn = running_node;
    rn->request_stop();
}

void RunnableNode::run()
{
//#if __APPLE__
    // signal handling doesn't seem to work on mac...
//#else
    struct SignalStacker {
        SignalStacker(RunnableNode* n) {
            running_node = n;
            previous_interrupt_signal_handler = ::signal(SIGINT, interrupt_signal_handler);
        }
        ~SignalStacker() {
            running_node = NULL;
            ::signal(SIGINT, previous_interrupt_signal_handler);
            previous_interrupt_signal_handler = 0;
        }

        // sighandler_t is GNU only I guess...
        // typedef void (*sighandler_t)(int);
        //sighandler_t previous_interrupt_signal_handler;
        void (*previous_interrupt_signal_handler)(int);
    };

    // Set myself as the signal handler
    SignalStacker s = SignalStacker(this);
//#endif

    this->stop_signal = false;

    // and let's go - just run in this thread
    child_thread_fn();
}

string RunnableNode::to_string() const
{
    std::stringstream sst;
    sst << "<RunnableNode " << Node::to_string() << ">";
    return sst.str();
}

MessagePtr RunnableNode::handle_rpc(MessagePtr rpc_message)
{
    if (rpc_message->module_name() == CoreModuleName) {

        const auto message_name = rpc_message->message_name();

        if (message_name == StartMessageName) {
            start();
            return make_shared<BlankMessage>(OKMessageName);
        }
        else if (message_name == StopMessageName) {
            stop();
            return make_shared<BlankMessage>(OKMessageName);
        } else {
            return Node::handle_rpc(rpc_message);
        }
    }

    return nullptr;
}

} // roboflex::core
