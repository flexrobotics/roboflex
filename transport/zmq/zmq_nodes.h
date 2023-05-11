#ifndef ROBOFLEX_TRANSPORT_ZMQ_NODES__H
#define ROBOFLEX_TRANSPORT_ZMQ_NODES__H

#include <memory>
#include <zmq.hpp>
#include "core/core.h"

namespace roboflex {
namespace transportzmq {

using std::cout, std::string, std::list, core::Node, core::RunnableNode, core::MessagePtr;

using ZMQContext = shared_ptr<zmq::context_t>;

inline ZMQContext MakeZMQContext(unsigned int num_io_threads = 1) {
    cout << "Made a new ZMQ Context from zmq version"
         << ZMQ_VERSION_MAJOR << "."
         << ZMQ_VERSION_MINOR << "."
         << ZMQ_VERSION_PATCH << endl;
    return make_shared<zmq::context_t>(num_io_threads);
}

/// A list of NICs i.e: { "inproc://sometqueuename", "ipc://somesocketname", "tcp://*:5432", etc }
typedef list<string> BindList;

/**
 * A node that publishes messages to a ZMQ socket. 
 */
class ZMQPublisher: public Node {
public:
    ZMQPublisher(
        ZMQContext context,
        const BindList& bind_addresses,
        const string& name = "ZMQPublisher",
        unsigned int max_queued_msgs = 1000);

    // A convenience constructor to bind to a single address.
    ZMQPublisher(
        ZMQContext context,
        const string& bind_address,
        const string& name = "ZMQPublisher",
        unsigned int max_queued_msgs = 1000):
            ZMQPublisher(context, BindList{bind_address}, name, max_queued_msgs) {}

    void receive(MessagePtr m) override;
    void publish(MessagePtr m) { this->signal_self(m); }

    const BindList & get_bind_addresses() const { return bind_addresses; }
    const unsigned int get_max_queued_msgs() const { return max_queued_msgs; }

protected:

    void ensure_zmq_socket();

    ZMQContext context;
    BindList bind_addresses;
    unique_ptr<zmq::socket_t> socket;
    unsigned int max_queued_msgs;
};

using ZMQPublisherPtr = shared_ptr<ZMQPublisher>;

/**
 * A node that subscribes to a ZMQ socket and signals messages
 * it receives from it. Must be start()-ed.
 */
class ZMQSubscriber: public RunnableNode {
public:
    ZMQSubscriber(
        ZMQContext context,
        const BindList& connect_addresses,
        const string& name = "ZMQSubscriber",
        unsigned int max_queued_msgs = 1000,
        unsigned int timeout_milliseconds = 10);

    // a convenience constructor for a single connection address
    ZMQSubscriber(
        ZMQContext context,
        const string& connect_address,
        const string& name = "ZMQSubscriber",
        unsigned int max_queued_msgs = 1000,
        unsigned int timeout_milliseconds = 10):
            ZMQSubscriber(context, BindList{connect_address}, name, max_queued_msgs, timeout_milliseconds) {}

    virtual ~ZMQSubscriber();

    core::MessagePtr pull(int timeout_milliseconds=10);
    void produce(int timeout_millisecond=10);

    string get_connect_address() const { return connect_addresses.front(); }
    BindList get_connect_addresses() const { return connect_addresses; }
    const unsigned int get_timeout_milliseconds() const { return _timeout_milliseconds; }
    const unsigned int get_max_queued_msgs() const { return max_queued_msgs; }

protected:
    void ensure_sockets();
    void destroy_sockets();

    void child_thread_fn() override;

    ZMQContext context;
    BindList connect_addresses;
    vector<shared_ptr<zmq::socket_t>> sockets;
    unsigned int max_queued_msgs;
    unsigned int _timeout_milliseconds;
    vector<zmq::pollitem_t> pollable_socket_items;
    bool sockets_constructed;
};

using ZMQSubscriberPtr = shared_ptr<ZMQSubscriber>;


} // namespace transportzmq
} // namespace roboflex

#endif // ROBOFLEX_TRANSPORT_ZMQ_NODES__H

