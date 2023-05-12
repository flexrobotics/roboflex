#include <sstream>
#include <signal.h>
#include "node.h"
#include "core_messages/core_messages.h"

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
        << " guid: " << get_guid() 
        << ">";
    return sst.str();
}


// --- Connection management ---

Node::NodePtr Node::connect(Node::NodePtr node)
{
    const std::lock_guard<std::recursive_mutex> lock(observer_collection_mutex);
    observers.push_back(node);
    node->on_connect(*this, true);
    this->on_connect(*node, false);
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
    node.on_connect(*this, true);
    this->on_connect(node, false);
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

void Node::receive_from(MessagePtr m, const Node& from)
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
    if (this->my_thread == nullptr) {
        auto f = [](std::stop_token stoken, RunnableNode* self) {
            self->stop_token = stoken;
            self->child_thread_fn();
        };
        this->my_thread.reset(new std::jthread(f, this));
    }
}

void RunnableNode::stop_and_join()
{
    if (this->my_thread != nullptr) {
        this->my_thread->request_stop();
        this->my_thread->join();
        this->my_thread.reset();
    }
}

void RunnableNode::request_stop()
{
    if (this->my_thread != nullptr) {
        this->my_thread->request_stop();
    }
}



// We switch the signal handler if run() is called. When run()
// is called, we simply run start_thread_fn() inside the current
// thread, without launching another thread - we assume we are 
// in the main thread of the process.
std::atomic<RunnableNode*> running_node = NULL;

void interrupt_signal_handler(int sig)
{
    RunnableNode *rn = running_node;
    rn->request_stop();
}

void RunnableNode::run()
{
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
        sighandler_t previous_interrupt_signal_handler;
    };

    // Set myself as the signal handler
    SignalStacker s = SignalStacker(this);

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
