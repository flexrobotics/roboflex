#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/complex.h>
#include "pybindings.h"
#include "core/core.h"
#include "core/util/uuid.h"
#include "core/util/utils.h"
#include "core/core_nodes/core_nodes.h"

#define FORCE_IMPORT_ARRAY                // numpy C api loading
#include <xtensor-python/pyarray.hpp>
#include <xtensor-python/pytensor.hpp>

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::core;
using namespace roboflex::nodes;


// Allows inheritance in Python from Node
class PyNode: public Node {
public:
    using Node::Node;

    /* Trampoline (need one for each virtual function) */
    void receive(MessagePtr m) override {
        // https://pybind11.readthedocs.io/en/stable/advanced/misc.html#global-interpreter-lock-gil
        // py::gil_scoped_acquire acquire;
        PYBIND11_OVERRIDE(
            void,      /* Return type */
            Node,      /* Parent class */
            receive,   /* Name of function in C++ (must match Python name) */
            m          /* Argument(s) */
        );
    }

    void receive_from(MessagePtr m, const Node& from) override {
        PYBIND11_OVERRIDE(void, Node, receive_from, m, from);
    }

    virtual NodePtr connect(NodePtr node) override {
        PYBIND11_OVERRIDE(NodePtr, Node, connect, node);
    }

    virtual void disconnect(NodePtr node) override {
        PYBIND11_OVERRIDE(void, Node, disconnect, node);
    }

    virtual string to_string() const override {
        PYBIND11_OVERRIDE(string, Node, to_string, );
    }

    MessagePtr handle_rpc(MessagePtr rpc_message) override {
        PYBIND11_OVERRIDE(MessagePtr, Node, handle_rpc, rpc_message);
    }
};

// Allows inheritance in Python from RunnableNode
class PyRunnableNode: public RunnableNode {
public:
    using RunnableNode::RunnableNode;

    void child_thread_fn() override {
        PYBIND11_OVERRIDE(void, RunnableNode, child_thread_fn, );
    }

    void start() override {
        PYBIND11_OVERRIDE(void, RunnableNode, start, );
    }

    void stop() override {
        PYBIND11_OVERRIDE(void, RunnableNode, stop, );
    }    /* Trampoline (need one for each virtual function) */
 
    void receive(MessagePtr m) override {
        PYBIND11_OVERRIDE(void, RunnableNode, receive, m);
    }

    void receive_from(MessagePtr m, const Node& from) override {
        PYBIND11_OVERRIDE(void, RunnableNode, receive_from, m, from);
    }

    virtual NodePtr connect(NodePtr node) override {
        PYBIND11_OVERRIDE(NodePtr, RunnableNode, connect, node);
    }

    virtual void disconnect(NodePtr node) override {
        PYBIND11_OVERRIDE(void, RunnableNode, disconnect, node);
    }

    virtual string to_string() const override {
        PYBIND11_OVERRIDE(string, RunnableNode, to_string, );
    }

    MessagePtr handle_rpc(MessagePtr rpc_message) override {
        PYBIND11_OVERRIDE(MessagePtr, RunnableNode, handle_rpc, rpc_message);
    }

};

// Allows inheritance in Python from Message
class PyMessage: public Message {
public:
    using Message::Message;

    std::string to_string() const override {
        PYBIND11_OVERRIDE(std::string, Message, to_string, );
    }
};


// A special case:
// Allows inheritance in Python from FrequencyGenerator,
// so that child nodes can just 'be' a frequency generator
// if they want.
class PyFrequencyGenerator: public FrequencyGenerator {
public:
    using FrequencyGenerator::FrequencyGenerator;

    void on_trigger(double wall_clock_time) override {
        PYBIND11_OVERRIDE(void, FrequencyGenerator, on_trigger, wall_clock_time);
    }

    void receive(MessagePtr m) override {
        PYBIND11_OVERRIDE(void, FrequencyGenerator, receive, m);
    }
};


PYBIND11_MODULE(roboflex_core_python_ext, m) {
    m.doc() = "ROBOFLEX!!!";

    xt::import_numpy();

    py::class_<sole::uuid>(m, "SoleUUID")
        .def("__eq__", &sole::uuid::operator==)
        .def("__neq__", &sole::uuid::operator!=)
        .def("__lt__", &sole::uuid::operator<)
        .def("__repr__", &sole::uuid::str)
    ;

    py::class_<MessageBackingStore, std::shared_ptr<MessageBackingStore>>(m, "MessageBackingStore")
        // Get the raw data, super dangerous! Do we want the const one? Is that even possible?
        .def("data", static_cast<uint8_t* (MessageBackingStore::*)()>(&MessageBackingStore::get_data))
        .def("size", &MessageBackingStore::get_size)
        .def("__repr__", &MessageBackingStore::to_string)
        .def_property_readonly("bytes", [](std::shared_ptr<MessageBackingStore> v) {
            const char* data = (const char*)(v->get_raw_data()); // should be get_raw_data?
            return py::bytes(data, v->get_raw_size()); // should be get_raw_size?
        })
    ;

    py::class_<MessageBackingStoreVector, MessageBackingStore, std::shared_ptr<MessageBackingStoreVector>>(m, "MessageBackingStoreVector")
        .def_static("copy_from", [] (py::bytes &bytes) {
            py::buffer_info info(py::buffer(bytes).request());
            const uint8_t *data = reinterpret_cast<const uint8_t *>(info.ptr);
            size_t length = static_cast<size_t>(info.size);
            auto v = std::make_shared<MessageBackingStoreVector>(data, length);
            v->blit_header();
            return v;
        }, py::call_guard<py::gil_scoped_release>())
    ;

    py::class_<Message, PyMessage, MessagePtr>(m, "Message")
        .def(py::init<const string&, const string&, shared_ptr<MessageBackingStore>>(),
            py::arg("module_name"),
            py::arg("message_name"),
            py::arg("backing_store"))
        .def(py::init<Message&, const std::string&>(),
            py::arg("other"),
            py::arg("new_name") = "")
        .def_property_readonly("module_name", &Message::module_name)
        .def_property_readonly("message_name", &Message::message_name)
        .def_property_readonly("source_node_name", &Message::source_node_name)
        .def_property_readonly("source_node_guid", &Message::source_node_guid)
        .def_property_readonly("message_counter", &Message::message_counter)
        .def_property_readonly("timestamp", &Message::timestamp)
        .def("set_timestamp", &Message::set_timestamp)
        .def("set_message_counter", &Message::set_message_counter)
        //.def("reset_for_send", &Message::reset_for_send)
        .def_property_readonly("payload", &Message::payload)
        .def("to_string", &Message::to_string)
        .def("__repr__", &Message::to_string)
    ;

    // py::class_<SuperCoreNode, std::shared_ptr<SuperCoreNode>>(m, "SuperCoreNode")
    //     .def("true_connect", (std::shared_ptr<SuperCoreNode> (SuperCoreNode::*) (std::shared_ptr<SuperCoreNode>)) &Node::true_connect, py::keep_alive<1, 2>(), py::call_guard<py::gil_scoped_release>())
    //     .def("true_disconnect", (void (SuperCoreNode::*) (std::shared_ptr<SuperCoreNode>)) &Node::true_disconnect, py::call_guard<py::gil_scoped_release>())
    //     .def("connect", (std::shared_ptr<SuperCoreNode> (SuperCoreNode::*) (std::shared_ptr<SuperCoreNode>)) &Node::connect, py::keep_alive<1, 2>(), py::call_guard<py::gil_scoped_release>())
    //     .def("disconnect", (void (SuperCoreNode::*) (std::shared_ptr<SuperCoreNode>)) &Node::disconnect, py::call_guard<py::gil_scoped_release>())
    //     .def("has_observers", &SuperCoreNode::has_observers, py::call_guard<py::gil_scoped_release>())
    //     .def("num_observers", &SuperCoreNode::num_observers, py::call_guard<py::gil_scoped_release>())
    //     .def("find_node_by_name", &SuperCoreNode::find_node_by_name, py::call_guard<py::gil_scoped_release>())
    //     .def("get_observers", &SuperCoreNode::get_observers, py::call_guard<py::gil_scoped_release>())
    //     .def("run", &SuperCoreNode::run, py::call_guard<py::gil_scoped_release>())
    //     .def("signal_stop_thread_no_join", &SuperCoreNode::signal_stop_thread_no_join, py::call_guard<py::gil_scoped_release>())
    //     .def("stop_signalled", &SuperCoreNode::stop_signalled)
    //     .def("get_thread_name", &SuperCoreNode::get_thread_name)
    //     .def("set_thread_name", &SuperCoreNode::set_thread_name)
    //     .def("print_graph", &SuperCoreNode::print_graph, py::arg("tab_level")=0)
    // ;

    py::class_<Node, PyNode, NodePtr>(m, "Node", py::dynamic_attr())
        .def(py::init<const std::string&>(),
            "Everything is a node!",
            py::arg("name") = "Node")

        .def_property_readonly("guid", [](std::shared_ptr<Node> n){ return n->get_guid().str(); })
        .def_property_readonly("name", &Node::get_name)

        .def("connect", (std::shared_ptr<Node> (Node::*) (std::shared_ptr<Node>)) &Node::connect, py::keep_alive<1, 2>(), py::call_guard<py::gil_scoped_release>())
        .def("disconnect", (void (Node::*) (std::shared_ptr<Node>)) &Node::disconnect, py::call_guard<py::gil_scoped_release>())
        .def("has_observers", &Node::has_observers)
        .def("num_observers", &Node::num_observers)
        .def("get_observers", &Node::get_observers)

        .def("receive_from", static_cast<void (Node::*)(MessagePtr, const Node&)>(&Node::receive_from), py::call_guard<py::gil_scoped_release>())
        .def("receive", static_cast<void (Node::*)(MessagePtr)>(&Node::receive), py::call_guard<py::gil_scoped_release>())

        .def("signal", &Node::signal, py::call_guard<py::gil_scoped_release>())
        .def("signal", [](std::shared_ptr<Node> a, py::object m) {
            a->signal(dynoflex_from_object(m));
        })
        // Keep these around for now...
        //}, py::keep_alive<0, 2>())
        //}, py::keep_alive<1, 2>(), py::call_guard<py::gil_scoped_release>())

        .def("signal_self", &Node::signal_self, py::call_guard<py::gil_scoped_release>())
        .def("signal_self", [](std::shared_ptr<Node> a, py::object m) {
            a->signal_self(dynoflex_from_object(m));
        })

        .def("__repr__", &Node::to_string)

        // let's try some syntactic sugar:

        // "node1 > node2", means "node1.connect(node2)", and return node 2
        .def("__gt__", [](std::shared_ptr<Node> a, std::shared_ptr<Node> b) {
            a->connect(b);
            return b;
        }, py::keep_alive<1, 2>()) //, py::call_guard<py::gil_scoped_release>())

        // "node1 >= node2" means connect node1 to node 2, and return node1
        .def("__ge__", [](std::shared_ptr<Node> a, std::shared_ptr<Node> b) {
            a->connect(b);
            return a;
        }, py::keep_alive<1, 2>()) //, py::call_guard<py::gil_scoped_release>())

        // "node1 >> [node2, node3]" means connect node1 to nodes 2 and 3, and return node1
        .def("__rshift__", [](std::shared_ptr<Node> a, const std::vector<std::shared_ptr<Node>>& l) {
            for (auto b: l) {
                a->connect(b);
            }
            return a;
        }, py::keep_alive<1, 2>()) //, py::call_guard<py::gil_scoped_release>())
    ;

    py::class_<RunnableNode, PyRunnableNode, std::shared_ptr<RunnableNode>>(m, "RunnableNode", py::dynamic_attr())
        .def(py::init<const std::string&>(),
            "Everything is a node!",
            py::arg("name") = "RunnableNode")

        .def_property_readonly("name", &RunnableNode::get_name)

        .def("start", &RunnableNode::start, py::call_guard<py::gil_scoped_release>())
        .def("stop", &RunnableNode::stop, py::call_guard<py::gil_scoped_release>())
        .def("request_stop", &RunnableNode::request_stop, py::call_guard<py::gil_scoped_release>())
        .def("stop_requested", &RunnableNode::stop_requested)       .def_property_readonly("guid", [](std::shared_ptr<Node> n){ return n->get_guid().str(); })

        .def("connect", (std::shared_ptr<Node> (RunnableNode::*) (std::shared_ptr<Node>)) &RunnableNode::connect, py::keep_alive<1, 2>(), py::call_guard<py::gil_scoped_release>())
        .def("disconnect", (void (RunnableNode::*) (std::shared_ptr<Node>)) &RunnableNode::disconnect, py::call_guard<py::gil_scoped_release>())
        .def("has_observers", &RunnableNode::has_observers)
        .def("num_observers", &RunnableNode::num_observers)
        .def("get_observers", &RunnableNode::get_observers)

        .def("receive_from", static_cast<void (RunnableNode::*)(MessagePtr, const Node&)>(&RunnableNode::receive_from), py::call_guard<py::gil_scoped_release>())
        .def("receive", static_cast<void (RunnableNode::*)(MessagePtr)>(&RunnableNode::receive), py::call_guard<py::gil_scoped_release>())

        .def("signal", &RunnableNode::signal, py::call_guard<py::gil_scoped_release>())
        .def("signal", [](std::shared_ptr<RunnableNode> a, py::object m) {
            a->signal(dynoflex_from_object(m));
        })
        // Keep these around for now...
        //}, py::keep_alive<0, 2>())
        //}, py::keep_alive<1, 2>(), py::call_guard<py::gil_scoped_release>())

        .def("signal_self", &RunnableNode::signal_self, py::call_guard<py::gil_scoped_release>())
        .def("signal_self", [](std::shared_ptr<RunnableNode> a, py::object m) {
            a->signal_self(dynoflex_from_object(m));
        })

        .def("__repr__", &RunnableNode::to_string)

        // let's try some syntactic sugar:

        // "node1 > node2", means "node1.connect(node2)", and return node 2
        .def("__gt__", [](std::shared_ptr<RunnableNode> a, std::shared_ptr<Node> b) {
            a->connect(b);
            return b;
        }, py::keep_alive<1, 2>()) //, py::call_guard<py::gil_scoped_release>())

        // "node1 >= node2" means connect node1 to node 2, and return node1
        .def("__ge__", [](std::shared_ptr<RunnableNode> a, std::shared_ptr<Node> b) {
            a->connect(b);
            return a;
        }, py::keep_alive<1, 2>()) //, py::call_guard<py::gil_scoped_release>())

        // "node1 >> [node2, node3]" means connect node1 to nodes 2 and 3, and return node1
        .def("__rshift__", [](std::shared_ptr<RunnableNode> a, const std::vector<std::shared_ptr<Node>>& l) {
            for (auto b: l) {
                a->connect(b);
            }
            return a;
        }, py::keep_alive<1, 2>()) //, py::call_guard<py::gil_scoped_release>())


    ;


    // ---------- Core messages -----------

    py::class_<BlankMessage, Message, std::shared_ptr<BlankMessage>>(m, "BlankMessage")
        .def(py::init<Message&>())
        .def(py::init<const string&>(),
            py::arg("message_name"))
    ;

    py::class_<StringMessage, Message, std::shared_ptr<StringMessage>>(m, "StringMessage")
        .def(py::init<Message&>())
        .def(py::init<const string&, const string&>(),
            py::arg("message_name"),
            py::arg("message"))
        .def("message", &StringMessage::message)
    ;

    py::class_<FloatMessage, Message, std::shared_ptr<FloatMessage>>(m, "FloatMessage")
        .def(py::init<Message&>())
        .def(py::init<const string&, const float>(),
            py::arg("message_name"),
            py::arg("value"))
        .def("value", &FloatMessage::value)
    ;

    py::class_<DoubleMessage, Message, std::shared_ptr<DoubleMessage>>(m, "DoubleMessage")
        .def(py::init<Message&>())
        .def(py::init<const string&, const double>(),
            py::arg("message_name"),
            py::arg("value"))
        .def("value", &DoubleMessage::value)
    ;


    // ---------- Utility nodes -----------

    py::class_<FrequencyGenerator, PyFrequencyGenerator, Node, std::shared_ptr<FrequencyGenerator>>(m, "FrequencyGenerator")
        .def(py::init<const float, const std::string &>(),
            "Create a Frequency Generator node. Be sure to call start()!",
            py::arg("frequency_hz") = 1.0,
            py::arg("name") = "FrequencyGenerator")
        .def("set_frequency", &FrequencyGenerator::set_frequency)
        .def("get_frequency", &FrequencyGenerator::get_frequency)
    ;

    py::class_<MessagePrinter, Node, std::shared_ptr<MessagePrinter>>(m, "MessagePrinter")
        .def(py::init<const std::string &>(),
            "Create a node that just prints messages to cout.",
            py::arg("name") = "MessagePrinter")
    ;


    // ---------- FRP Utility nodes -----------

    py::class_<CallbackFun, Node, std::shared_ptr<CallbackFun>>(m, "CallbackFun")
        .def(py::init<CallbackFun::CallbackFunction, const std::string &>(),
            "Create a CallbackFun node.",
            py::arg("callback_function"),
            py::arg("name") = "CallbackFun")
    ;

    py::class_<FilterFun, Node, std::shared_ptr<FilterFun>>(m, "FilterFun")
        .def(py::init<FilterFun::FilterFunction, const std::string &>(),
            "Create a FilterFun node, which filters according to the given predicate.",
            py::arg("filter_function"),
            py::arg("name") = "FilterFun")
    ;

    py::class_<FilterName, Node, std::shared_ptr<FilterName>>(m, "FilterName")
        .def(py::init<const std::string&, const std::string&>(),
            "Create a FilterName node, which filters by message name.",
            py::arg("message_name"),
            py::arg("name") = "FilterName")
    ;

    py::class_<FilterNamePassthrough, Node, std::shared_ptr<FilterNamePassthrough>>(m, "FilterNamePassthrough")
        .def(py::init<const std::string&, bool, const std::string&>(),
            "Create a FilterNamePassthrough node, which filters by message name.",
            py::arg("message_name"),
            py::arg("initial_passthrough"),
            py::arg("name") = "FilterNamePassthrough")
        .def("set_passthrough", &FilterNamePassthrough::set_passthrough)
        .def("get_passthrough", &FilterNamePassthrough::get_passthrough)
    ;

    py::class_<MapFun, Node, std::shared_ptr<MapFun>>(m, "MapFun")
        .def(py::init<MapFun::MapFunction, const std::string &>(),
            "Create a MapFun node.",
            py::arg("map_function"),
            py::arg("name") = "MapFun")
    ;


    // ------------ utilities ------------

    m.def("get_current_time", &get_current_time, "Gets the current time the same way that everything else in roboflex does.");
    m.def("get_roboflex_core_version", &get_roboflex_core_version, "Gets the current roboflex version.");
    m.def("initialize_module_loading", &pybind11::detail::initialize_module_loading, "Initializes module loading. Must be called from the main thread.");
}
