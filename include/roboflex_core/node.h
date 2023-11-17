#ifndef ROBOFLEX_CORE_NODE__H
#define ROBOFLEX_CORE_NODE__H

#include <memory>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include "message.h"
#include "util/uuid.h"

namespace roboflex::core {

using std::string, std::shared_ptr, std::ostream, std::list, std::set, sole::uuid;

/**
 * A Node is a basic unit of computation. It can be connected to other nodes,
 * and it can signal and receive messages. Reception is done via inheritance.
 */
class Node {
public:
    using NodePtr = shared_ptr<Node>;

    Node(const string& name = "");
    virtual ~Node();

    virtual string to_string() const;
    string graph_to_string(int level=0) const;


    // --- Node identification ---

    const string& get_name() const { return name; }
    const uuid& get_guid() const { return guid; }


    // --- Connection management ---

    // Nodes can connect to other nodes
    virtual NodePtr connect(NodePtr node);
    virtual Node& connect(Node &node);
    virtual void disconnect(NodePtr node);
    virtual void disconnect(Node &node);
    bool has_observers() const;
    size_t num_observers() const;
    list<NodePtr> get_observers() const;

    // Sugar for .connect
    Node& operator > (Node& other);
    NodePtr operator > (NodePtr other);

    // Various ways to walk nodes and connections. Callbacks
    // are called with the node and the depth from this node.
    using NodeWalkCallback = std::function<void(NodePtr, int)>;
    void walk_nodes(NodeWalkCallback node_fun, bool forwards) const;
    void walk_nodes_forwards(NodeWalkCallback node_fun) const;
    void walk_nodes_backwards(NodeWalkCallback node_fun) const;

    // Callback is called with parent node, child node, depth of parent.
    using ConnectionWalkCallback = std::function<void(NodePtr, NodePtr, int)>;
    void walk_connections(ConnectionWalkCallback connection_fun, bool forwards) const;
    void walk_connections_forwards(ConnectionWalkCallback connection_fun) const;
    void walk_connections_backwards(ConnectionWalkCallback connection_fun) const;

    // Graph is pruned according to filter predicate. Predicate
    // callback is called with each node and its depth.
    using NodeFilterCallback = std::function<bool(NodePtr, int)>;
    void filter_nodes(NodeFilterCallback filter_fun);


    // --- Signal and receive methods. ---

    // Signalling sends messages to nodes I connect to.
    MessagePtr signal(MessagePtr m);
    MessagePtr signal_self(MessagePtr m);

    // Children may override receive methods, in order to use
    // signalled messages.
    virtual void receive_from(MessagePtr m, const Node& from);
    virtual void receive(MessagePtr m);

 
    // --- RPC --- 

    virtual MessagePtr handle_rpc(MessagePtr rpc_message);

protected:

    // Every node has a name, but it is optional, 
    // and does not have to be unique.
    string name;

    // Every node has a unique identifier.
    uuid guid;

    // Every node has a list of observers. We protect this 
    // list with a mutex. We can pay the cost of a mutex,
    // because we signal messages at a few kHz at most.
    list<NodePtr> observers;
    mutable std::recursive_mutex observer_collection_mutex;

    // locks mutex, incs count, calls receive on all observers or just on me
    void notify_observers(MessagePtr m);
    void notify_self(MessagePtr m);

    // Used for out-of-order tracking and metrics.
    uint64_t message_send_counter = 0;

    // called when I get connected to a node, both ways (whether I am the parent or child).
    virtual void on_connect(const Node&, bool) {}
};


/**
 * A RunnableNode is a Node that can be 'start()'-ed:
 * it runs 'child_thread_fn' in its own thread. Child 
 * classes of Node should override this instead if they
 * want to run in a thread.
 */
class RunnableNode : public Node {
public:
    RunnableNode(const string& name = "");
    virtual ~RunnableNode();

    string to_string() const override;

    virtual void start();
    virtual void stop_and_join();
    virtual void stop() { this->stop_and_join(); }
    virtual void request_stop();
    
    //bool stop_requested() const { return my_thread ? stop_token.stop_requested() : false; }
    //bool stop_requested() const { return my_thread ? stop_signal.load() : false; }
    bool stop_requested() const { return stop_signal.load(); }

    // Calls start_thread_fn in the current thread (does not launch
    // another thread).
    void run();

    MessagePtr handle_rpc(MessagePtr rpc_message) override;

protected:

    // clang doesn't support jthread yet :(
    // std::unique_ptr<std::jthread> my_thread = nullptr;
    // std::stop_token stop_token;

    std::unique_ptr<std::thread> my_thread = nullptr;
    std::atomic<bool> stop_signal = {true};

    // Child classes should override this 
    // method to run in a thread.
    virtual void child_thread_fn() {}
};

using NodePtr = Node::NodePtr;
using RunnableNodePtr = shared_ptr<RunnableNode>;

} // roboflex::core

#endif // ROBOFLEX_CORE_NODE__H
