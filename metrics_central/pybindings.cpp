#include <pybind11/pybind11.h>
#include "core/core.h"
#include "profiler.h"

namespace py  = pybind11;
using namespace roboflex::profiling;

PYBIND11_MODULE(roboflex_profiler_ext, m) {

    py::class_<Profiler, roboflex::nodes::GraphController, std::shared_ptr<Profiler>>(m, "Profiler")
        .def(py::init<const std::string&, const int, const std::string&, const float, const std::string&>(),
            "Create a Profiler that publishes metrics to an mqtt broker.",
            py::arg("mqtt_broker_address") = "127.0.0.1",
            py::arg("mqtt_broker_port") = 1883,
            py::arg("mqtt_metrics_topic") = "roboflexmetrics",
            py::arg("metrics_publishing_frequency_hz") = 1.0,
            py::arg("name") = "Profiler")
        .def(py::init<roboflex::core::NodePtr, const float, const std::string&>(),
            "Create a Profiler with a custom metrics publisher.",
            py::arg("metrics_publisher"),
            py::arg("metrics_publishing_frequency_hz") = 1.0,
            py::arg("name") = "Profiler")
        .def("start", &Profiler::start)
    ;
}
