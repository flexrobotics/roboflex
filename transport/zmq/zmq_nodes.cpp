#include "transport/zmq/zmq_nodes.h"
#include "core/message_backing_store.h"
#include "core/util/utils.h"

namespace roboflex {
namespace transportzmq {


// -- ZMQPublisher --

ZMQPublisher::ZMQPublisher(
    ZMQContext context,
    const BindList &bind_addresses,
    const std::string &name,
    unsigned int max_queued_msgs):
        Node(name),
        context(context),
        bind_addresses(bind_addresses),
        max_queued_msgs(max_queued_msgs)
{

}

void ZMQPublisher::ensure_zmq_socket() 
{
    if (socket == nullptr) {
        socket = std::make_unique<zmq::socket_t>(*context, ZMQ_PUB);
        socket->setsockopt(ZMQ_SNDHWM, this->max_queued_msgs);
        for (auto bind_address: bind_addresses) {
            socket->bind(bind_address);
        }
    }
}

void ZMQPublisher::receive(MessagePtr m)
{
    ensure_zmq_socket();

    if (m == nullptr) {
        return;
    }

    // create a zmq message and send it, tracking the
    // payload for 0-copy
    auto z_message = zmq::message_t(
        m->get_raw_data(),
        m->get_raw_size(),
        core::MessageBackingStore::raw_data_deletion_function,
        new std::shared_ptr<core::MessageBackingStore>(m->payload()));

    // perform the actual send
    socket->send(z_message, zmq::send_flags::none);

    // propagate
    signal(m);
}


// -- For 0-copy in subscribe: implement our own MessageBackingStore type. --
// Notice that when this is constructed, a zmq message is MOVED in.
// So when this gets destructed, it will delete the message,
// which contains the bytes of the actual message.

struct MessageBackingStoreZMQ: public core::MessageBackingStore
{
    MessageBackingStoreZMQ(zmq::message_t && z_msg):
        z_msg(std::move(z_msg)) {}

    uint8_t* get_raw_data() override { return (uint8_t*)(z_msg.data()); }
    const uint8_t* get_raw_data() const override { return (const uint8_t*)(z_msg.data()); }
    uint32_t get_raw_size() const override { return z_msg.size(); }

    void print_on(ostream& os) const override {
        os << "<MessageBackingStoreZMQ bytes: " << get_size() << ">";
    }

    zmq::message_t z_msg;
};


// -- ZMQBaseSubscriber --

ZMQSubscriber::ZMQSubscriber(
    ZMQContext context,
    const BindList &connect_addresses,
    const std::string &name,
    unsigned int max_queued_msgs,
    unsigned int timeout_milliseconds):
        RunnableNode(name),
        context(context),
        connect_addresses(connect_addresses),
        max_queued_msgs(max_queued_msgs),
        _timeout_milliseconds(timeout_milliseconds),
        sockets_constructed(false)
{

}

ZMQSubscriber::~ZMQSubscriber()
{
    //destroy_sockets();
}

void ZMQSubscriber::ensure_sockets()
{
    if (!sockets_constructed) {

        // one subscribe socket for each address
        for (std::string connect_address: connect_addresses) {
            auto socket = std::shared_ptr<zmq::socket_t>(new zmq::socket_t(*context, ZMQ_SUB));
            socket->setsockopt(ZMQ_RCVHWM, max_queued_msgs);
            socket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
            socket->connect(connect_address);
            sockets.push_back(socket);
        }

        // construct the pollable item list
        for (auto& socket_shared : sockets) {
            zmq::socket_t *socket = socket_shared.get();
            pollable_socket_items.push_back({ static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0 });
        }

        sockets_constructed = true;
    }
}

void ZMQSubscriber::destroy_sockets()
{
    if (sockets_constructed) {
        pollable_socket_items.clear();
        sockets.clear();
        sockets_constructed = false;
    }
}

core::MessagePtr ZMQSubscriber::pull(int timeout_milliseconds)
{
    ensure_sockets();

    try {
        // poll all the sockets
        zmq::poll(pollable_socket_items.data(), sockets.size(), timeout_milliseconds);
    } catch (zmq::error_t& e) {
        // just bail
        return nullptr;
    }

    // check each socket
    for (size_t i=0; i<sockets.size(); i++) {

        if (pollable_socket_items[i].revents & ZMQ_POLLIN) {

            auto socket = sockets[i];

            // -- receive the message
            zmq::message_t z_message;
            auto z_nbytes = socket->recv(z_message);
            if (!z_nbytes.has_value()) continue;

            // only if we have any observers do we bother signalling.
            if (this->has_observers()) {

                // get a payload object that will, ultimately, delete the body_message
                auto payload = std::make_shared<MessageBackingStoreZMQ>(std::move(z_message));

                // construct and return the message
                auto m = std::make_shared<core::Message>(payload);

                return m;
            }
        }
    }

    return nullptr;
}

void ZMQSubscriber::produce(int timeout_milliseconds)
{
    auto m = pull(timeout_milliseconds);
    if (m != nullptr) {
        this->signal(m);
    }
}

void ZMQSubscriber::child_thread_fn()
{
    ensure_sockets();

    while (!this->stop_requested()) {
        auto m = pull(_timeout_milliseconds);
        if (m != nullptr) {
            signal(m);
        }
    }

    destroy_sockets();
}

} // namespace transportzmq
} // namespace roboflex
