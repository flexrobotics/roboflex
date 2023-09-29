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
#include "roboflex/util/jpeg/jpeg.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::utiljpeg;
using std::string;

PYBIND11_MODULE(roboflex_util_jpeg_ext, m) {
    m.doc() = "roboflex_util_jpeg_ext";

    py::class_<JPEGCompressor, core::Node, std::shared_ptr<JPEGCompressor>>(m, "JPEGCompressor")
        .def(py::init<const string &, const string&, const string &, const string &, bool>(),
            "Creates a JPEGCompressor.",
            py::arg("image_key") = "rgb",
            py::arg("output_key") = "jpeg",
            py::arg("filename_prefix") = "",
            py::arg("name") = "JPEGCompressor",
            py::arg("debug") = false)
    ;

    py::class_<JPEGDecompressor, core::Node, std::shared_ptr<JPEGDecompressor>>(m, "JPEGDecompressor")
        .def(py::init<const string &, const string&, const string &, bool>(),
            "Creates a JPEGCompressor.",
            py::arg("input_key") = "jpeg",
            py::arg("output_key") = "rgb",
            py::arg("name") = "JPEGDecompressor",
            py::arg("debug") = false)
    ;
}
