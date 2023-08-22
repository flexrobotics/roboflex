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
#include "util/png/png.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::utilpng;
using std::string;

PYBIND11_MODULE(roboflex_util_png_ext, m) {
    m.doc() = "roboflex_util_png_ext";

    py::class_<PNGCompressor, core::Node, std::shared_ptr<PNGCompressor>>(m, "PNGCompressor")
        .def(py::init<const string &, const string&, const string &, const string &, bool>(),
            "Creates a PNGCompressor.",
            py::arg("image_key") = "rgb",
            py::arg("output_key") = "png",
            py::arg("filename_prefix") = "",
            py::arg("name") = "PNGCompressor",
            py::arg("debug") = false)
    ;

    py::class_<PNGDecompressor, core::Node, std::shared_ptr<PNGDecompressor>>(m, "PNGDecompressor")
        .def(py::init<const string &, const string&, const string &, bool>(),
            "Creates a PNGDecompressor.",
            py::arg("input_key") = "jpeg",
            py::arg("output_key") = "rgb",
            py::arg("name") = "PNGDecompressor",
            py::arg("debug") = false)
    ;
}
