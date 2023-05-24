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

using std::string, std::shared_ptr, std::ostream, std::list, sole::uuid;

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
    virtual void on_connect(const Node& node, bool node_is_child) {}
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
    bool stop_requested() const { return my_thread ? stop_signal.load() : false; }

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

} // roboflex::core

#endif // ROBOFLEX_CORE_NODE__H
