#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/complex.h>
#include "roboflex_core/pybindings.h"
#include "roboflex_core/core.h"
#include "roboflex_core/util/uuid.h"
#include "roboflex_core/util/utils.h"
#include "roboflex_core/core_nodes/core_nodes.h"

#define FORCE_IMPORT_ARRAY                // numpy C api loading
#include <xtensor-python/pyarray.hpp>
#include <xtensor-python/pytensor.hpp>

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::core;
using namespace roboflex::nodes;


// A special case:
// Allows inheritance in Python from FrequencyGenerator,
// so that child nodes can just 'be' a frequency generator
// if they want (inherit on_trigger).
template <class FrequencyGeneratorBase = FrequencyGenerator> 
class PyFrequencyGenerator: public PyRunnableNode<FrequencyGeneratorBase> {
public:
    using PyRunnableNode<FrequencyGeneratorBase>::PyRunnableNode;

    void on_trigger(double wall_clock_time) override {
        PYBIND11_OVERRIDE(void, FrequencyGeneratorBase, on_trigger, wall_clock_time);
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


PYBIND11_MODULE(roboflex_core_python_ext, m) 
{
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
        }, py::call_guard<py::gil_scoped_acquire>()) // we must have it to copy the buffer
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
        .def_property_readonly("payload", &Message::payload)
        .def("set_timestamp", &Message::set_timestamp)
        .def("set_message_counter", &Message::set_message_counter)
        .def("to_string", &Message::to_string)
        .def("__repr__", &Message::to_string)
    ;

    py::class_<Node, PyNode<>, NodePtr>(m, "Node", py::dynamic_attr())
        .def(py::init<const std::string&>(),
            "Everything is a node!",
            py::arg("name") = "Node")

        .def_property_readonly("guid", [](std::shared_ptr<Node> n){ return n->get_guid().str(); })
        .def_property_readonly("name", &Node::get_name)
        .def("graph_to_string", &Node::graph_to_string,
            py::arg("level")=0)

        .def("walk_nodes", &Node::walk_nodes)
        .def("walk_nodes_forwards", &Node::walk_nodes_forwards)
        .def("walk_nodes_backwards", &Node::walk_nodes_backwards)
        .def("walk_connections", &Node::walk_connections)
        .def("walk_connections_forwards", &Node::walk_connections_forwards)
        .def("walk_connections_backwards", &Node::walk_connections_backwards)
        .def("filter_nodes", &Node::filter_nodes)

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

    py::class_<RunnableNode, Node, PyRunnableNode<>, std::shared_ptr<RunnableNode>>(m, "RunnableNode", py::dynamic_attr())
        .def(py::init<const std::string&>(),
            "Everything is a node!",
            py::arg("name") = "RunnableNode")
        .def("start", &RunnableNode::start, py::call_guard<py::gil_scoped_release>())
        .def("stop", &RunnableNode::stop, py::call_guard<py::gil_scoped_release>())
        .def("request_stop", &RunnableNode::request_stop, py::call_guard<py::gil_scoped_release>())
        .def("stop_requested", &RunnableNode::stop_requested)       .def_property_readonly("guid", [](std::shared_ptr<Node> n){ return n->get_guid().str(); })
        .def("run", &RunnableNode::run, py::call_guard<py::gil_scoped_release>())
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

    py::class_<FrequencyGenerator, RunnableNode, PyFrequencyGenerator<>, std::shared_ptr<FrequencyGenerator>>(m, "FrequencyGenerator")
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

    // py::class_<LastOne, Node, std::shared_ptr<LastOne>>(m, "LastOne")
    //     .def(py::init<const std::string &>(),
    //         "Create a node that just remembers the last message, in a thread-safe way.",
    //         py::arg("name") = "LastOne")
    //     .def_property_readonly("last_one", &LastOne::get_last_one)
    // ;


    // ---------- Metrics -----------

    py::class_<MetricTracker, std::shared_ptr<MetricTracker>>(m, "MetricTracker")
        .def(py::init<>(), "Creates a MetricsTracker object.")

        .def("record_value", &MetricTracker::record_value)
        .def("reset", &MetricTracker::reset)
        .def("pretty_print", &MetricTracker::pretty_print)

        .def_readonly("count", &MetricTracker::count)
        .def_readonly("total", &MetricTracker::total)
        .def_readonly("mean", &MetricTracker::mean_value)
        .def_property_readonly("variance", &MetricTracker::variance_value)
        .def_readonly("max", &MetricTracker::max_value)
        .def_readonly("min", &MetricTracker::min_value)

        .def("to_string", &MetricTracker::to_string)
        .def("to_pretty_string", &MetricTracker::to_pretty_string)
        .def("__repr__", &MetricTracker::to_string)
    ;

    py::class_<MetricsMessage, Message, std::shared_ptr<MetricsMessage>>(m, "MetricsMessage")
        .def(py::init<Message&>())
        .def_readonly("metrics", &MetricsMessage::metrics)
        .def_property_readonly("parent_node_name", &MetricsMessage::parent_node_name)
        .def_property_readonly("child_node_name", &MetricsMessage::child_node_name)
        .def_property_readonly("parent_node_guid", &MetricsMessage::parent_node_guid)
        .def_property_readonly("child_node_guid", &MetricsMessage::child_node_guid)
        .def_property_readonly("current_mem_usage", &MetricsMessage::current_mem_usage)
        .def_property_readonly("elapsed_time", &MetricsMessage::elapsed_time)
        .def_property_readonly("host_name", &MetricsMessage::host_name)
        .def("to_pretty_string", &MetricsMessage::to_pretty_string)
        .def("pretty_print", &MetricsMessage::pretty_print)
        .def("__repr__", &MetricsMessage::to_string)
    ;

    py::class_<MetricsPublisherNode, Node, std::shared_ptr<MetricsPublisherNode>>(m, "MetricsPublisherNode")
        .def(py::init<const std::string &>(),
            "Create a metrics publisher node",
            py::arg("name") = "MetricsPublisherNode")
        .def("publish_and_reset", &MetricsPublisherNode::publish_and_reset)
        .def("record_metrics", &MetricsPublisherNode::record_metrics)
    ;

    py::class_<MetricsNode, Node, std::shared_ptr<MetricsNode>>(m, "MetricsNode")
        .def(py::init<const std::string &, const float>(),
            "Create a MetricsNode node.",
            py::arg("name") = "MetricsNode",
            py::arg("passive_frequency_hz") = 0)
        .def(py::init<shared_ptr<Node>, const std::string &, const float>(),
            "Create a MetricsNode node.",
            py::arg("publisher_target_node"),
            py::arg("name") = "MetricsNode",
            py::arg("passive_frequency_hz") = 0)
        .def("pretty_print", &MetricsNode::pretty_print)
        .def("reset", &MetricsNode::reset)
        .def_readonly("publisher_node", &MetricsNode::publisher_node)
        .def_readonly("passive_frequency_hz", &MetricsNode::passive_frequency_hz)
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
        .def(py::init([](py::function f, const std::string &name) {
            auto l = [f](MessagePtr m){
                pybind11::gil_scoped_acquire acq;

                // Invoke the map function
                py::object o = f(m);

                // ... which must return a dict.
                if (!py::isinstance<py::dict>(o)) {
                    throw std::runtime_error("MapFun function must return a dict.");
                }

                // Return a dynoflex message to contain the result
                return pybind11::detail::dynoflex_from_object(o);
            };
            return std::make_shared<MapFun>(l, name);
        }),
            "Create a MapFun node.",
            py::arg("f"),
            py::arg("name") = "MapFun")
    ;


    // ------------ utilities ------------

    m.def("get_current_time", &get_current_time, "Gets the current time the same way that everything else in roboflex does.");
    m.def("get_roboflex_core_version", &get_roboflex_core_version, "Gets the current roboflex version.");
    //m.def("initialize_module_loading", &pybind11::detail::initialize_module_loading, "Initializes module loading. Must be called from the main thread.");
}
