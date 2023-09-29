#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "roboflex/core/core.h"
#include "roboflex/core/python/pybindings.h"
#include "roboflex/transport/tq/tq.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::transporttq;

PYBIND11_MODULE(roboflex_transport_tq_ext, m) {
    m.doc() = "roboflex_transport_tq_ext";

    py::class_<TQPubSub, core::RunnableNode, std::shared_ptr<TQPubSub>>(m, "TQPubSub")
        .def(py::init<const std::string &,
                      unsigned int,
                      unsigned int>(),
            "Creates a TQPubSub.",
            py::arg("name") = "TQPubSub",
            py::arg("max_queued_msgs") = 1000,
            py::arg("timeout_milliseconds") = 10)
    ;
}
