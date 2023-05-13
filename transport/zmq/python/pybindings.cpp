#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "core/core.h"
#include "core/python/pybindings.h"
#include "transport/zmq/zmq_nodes.h"


namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::transportzmq;


PYBIND11_MODULE(roboflex_transport_zmq_ext, m) {
    m.doc() = "roboflex_transport_zmq_ext";

    py::class_<zmq::context_t, std::shared_ptr<zmq::context_t>>(m, "ZMQContext")
        .def(py::init(&MakeZMQContext),
            "Create a zeromq context. Need for any subsequent zmq things.",
            py::arg("num_io_threads") = 1)
    ;

    py::class_<ZMQPublisher, core::Node, std::shared_ptr<ZMQPublisher>>(m, "ZMQPublisher")
        .def(py::init<ZMQContext,
                      const BindList&,
                      const std::string &,
                      unsigned int>(),
            "Creates a ZMQPublisher from a list of bind addresses.",
            py::arg("zmq_context"),
            py::arg("bind_addresses"),
            py::arg("name") = "ZMQPublisher",
            py::arg("max_queued_msgs") = 1000)
        .def(py::init<ZMQContext,
                      const std::string&,
                      const std::string&,
                      unsigned int>(),
            "Creates a ZMQPublisher from a single bind address.",
            py::arg("zmq_context"),
            py::arg("bind_address"),
            py::arg("name") = "ZMQPublisher",
            py::arg("max_queued_msgs") = 1000)
        .def_property_readonly("bind_addresses", &ZMQPublisher::get_bind_addresses)
        .def_property_readonly("max_queued_msgs", &ZMQPublisher::get_max_queued_msgs)
        .def("publish", &ZMQPublisher::publish)
        .def("publish", [](std::shared_ptr<ZMQPublisher> a, py::object m) {
            a->publish(dynoflex_from_object(m));
        })
    ;

    py::class_<ZMQSubscriber, core::RunnableNode, std::shared_ptr<ZMQSubscriber>>(m, "ZMQSubscriber")
        .def(py::init<ZMQContext,
                      const BindList&,
                      const std::string&,
                      unsigned int,
                      unsigned int>(),
            "Creates a ZMQSubscriber to a list of connect addresses.",
            py::arg("zmq_context"),
            py::arg("connect_addresses"),
            py::arg("name") = "ZMQSubscriber",
            py::arg("max_queued_msgs") = 1000,
            py::arg("timeout_milliseconds") = 10)

        .def(py::init<ZMQContext,
                      const std::string&,
                      const std::string&,
                      unsigned int,
                      unsigned int>(),
            "Creates a ZMQSubscriber to a single connect address.",
            py::arg("zmq_context"),
            py::arg("connect_address"),
            py::arg("name") = "ZMQSubscriber",
            py::arg("max_queued_msgs") = 1000,
            py::arg("timeout_milliseconds") = 10)

        .def("pull", &ZMQSubscriber::pull)
        .def("produce", &ZMQSubscriber::produce)

        .def_property_readonly("connect_addresses", &ZMQSubscriber::get_connect_addresses)
        .def_property_readonly("connect_address", &ZMQSubscriber::get_connect_address)
        .def_property_readonly("max_queued_msgs", &ZMQSubscriber::get_max_queued_msgs)
        .def_property_readonly("timeout_milliseconds", &ZMQSubscriber::get_timeout_milliseconds)
    ;
}
