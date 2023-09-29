#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "roboflex/core/core.h"
#include "roboflex/visualization/visualization.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::visualization;

PYBIND11_MODULE(roboflex_visualization_ext, m) {
    m.doc() = "roboflex_visualization_ext";

    py::class_<Television, nodes::FrequencyGenerator, std::shared_ptr<Television>>(m, "Television")
    ;

    py::class_<RGBImageTV, Television, std::shared_ptr<RGBImageTV>>(m, "RGBImageTV")
        .def(py::init<const float, size_t, size_t, const std::string&, const pair<int, int>&, const bool, const bool, const std::string&>(),
            "Create a graph node that shows in realtime an rgb data stream.",
            py::arg("frequency_hz"),
            py::arg("width") = 640,
            py::arg("height") = 480,
            py::arg("image_key") = "rgb",
            py::arg("initial_pos") = std::pair<int, int>(-1, -1),
            py::arg("mirror") = false,
            py::arg("debug") = false,
            py::arg("name") = "RGBImageTV")
    ;

    py::class_<DepthTV, Television, std::shared_ptr<DepthTV>>(m, "DepthTV")
        .def(py::init<const float, size_t, size_t, const std::string&, const pair<int, int>&, const bool, const bool, const std::string&>(),
            "Create a graph node that shows in realtime a depth data stream.",
            py::arg("frequency_hz"),
            py::arg("width") = 640,
            py::arg("height") = 480,
            py::arg("image_key") = "depth",
            py::arg("initial_pos") = std::pair<int, int>(-1, -1),
            py::arg("mirror") = false,
            py::arg("debug") = false,
            py::arg("name") = "DepthTV")
    ;

    py::class_<MaskColorizer, core::Node, std::shared_ptr<MaskColorizer>>(m, "MaskColorizer")
        .def(py::init<const std::string&, const std::string&, bool, const std::string&>(),
            "Create a graph node can turn a mask of (H,W,N) to colorized (H,W,3).",
            py::arg("input_key") = "rgb",
            py::arg("output_key") = "rgb",
            py::arg("render_on_input") = true,
            py::arg("name") = "MaskColorizer")
    ;
}
