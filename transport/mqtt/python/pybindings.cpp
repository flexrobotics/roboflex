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
#include "transport/mqtt/mqtt_nodes.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::transportmqtt;

PYBIND11_MODULE(roboflex_transport_mqtt_ext, m) {
    m.doc() = "roboflex_transport_mqtt_ext";

    py::class_<MQTTContext, std::shared_ptr<MQTTContext>>(m, "MQTTContext")
        .def(py::init(&MakeMQTTContext),
            "Create an mqtt context. Need for any subsequent mqtt things.")
    ;

    py::class_<MQTTNodeBase, std::shared_ptr<MQTTNodeBase>>(m, "MQTTNodeBase")
        .def_property_readonly("broker_address", &MQTTNodeBase::get_broker_address)
        .def_property_readonly("broker_port", &MQTTNodeBase::get_broker_port)
        .def_property_readonly("topic_name", &MQTTNodeBase::get_topic_name)
        .def_property_readonly("keepalive_seconds", &MQTTNodeBase::get_keepalive_seconds)
        .def_property_readonly("qos", &MQTTNodeBase::get_qos)
        .def_property_readonly("debug", &MQTTNodeBase::get_debug)
    ;

    py::class_<MQTTPublisher, core::Node, MQTTNodeBase, std::shared_ptr<MQTTPublisher>>(m, "MQTTPublisher")
        .def(py::init<MQTTContextPtr,
                      const std::string&,
                      int,
                      const std::string&,
                      const std::string&,
                      int,
                      int,
                      bool,
                      bool>(),
            "Creates an MQTTPublisher.",
            py::arg("mqtt_context"),
            py::arg("broker_address"),
            py::arg("broker_port"),
            py::arg("topic_name"),
            py::arg("name") = "MQTTPublisher",
            py::arg("keepalive_seconds") = 60,
            py::arg("qos") = 0,
            py::arg("retained") = false,
            py::arg("debug") = false)

        .def_property_readonly("retained", &MQTTPublisher::get_retained)
        .def("publish", &MQTTPublisher::publish)
        .def("publish", [](std::shared_ptr<MQTTPublisher> a, py::object m) {
            a->publish(dynoflex_from_object(m));
        })
    ;

    py::class_<MQTTSubscriber, core::RunnableNode, MQTTNodeBase, std::shared_ptr<MQTTSubscriber>>(m, "MQTTSubscriber")
        .def(py::init<MQTTContextPtr,
                      const std::string&,
                      int,
                      const std::string&,
                      const std::string&,
                      int,
                      int,
                      int,
                      bool>(),
            "Creates an MQTTSubscriber.",
            py::arg("mqtt_context"),
            py::arg("broker_address"),
            py::arg("broker_port"),
            py::arg("topic_name"),
            py::arg("name") = "MQTTSubscriber",
            py::arg("keepalive_seconds") = 60,
            py::arg("qos") = 0,
            py::arg("loop_timeout_milliseconds") = 100,
            py::arg("debug") = false)

        .def_property_readonly("loop_timeout_milliseconds", &MQTTSubscriber::get_loop_timeout_milliseconds)
    ;
}
