#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "roboflex/core/core.h"
#include "roboflex/webcam_uvc/webcam_uvc.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::core;
using namespace roboflex::webcam_uvc;


PYBIND11_MODULE(roboflex_webcam_uvc_ext, m) {
    m.doc() = "roboflex_webcam_uvc_ext";

    m.def("get_device_list", &get_device_list);
    m.def("get_device_list_string", &get_device_list_string);

    py::register_exception<WebcamException>(m, "WebcamException");

    py::enum_<uvc_frame_format>(m, "uvc_frame_format")
        .value("UVC_FRAME_FORMAT_ANY", UVC_FRAME_FORMAT_ANY)
        .value("UVC_FRAME_FORMAT_UNCOMPRESSED", UVC_FRAME_FORMAT_UNCOMPRESSED)
        .value("UVC_FRAME_FORMAT_COMPRESSED", UVC_FRAME_FORMAT_COMPRESSED)
        // YUYV/YUV2/YUV422: YUV encoding with one luminance value per pixel and
        // one UV (chrominance) pair for every two pixels.
        .value("UVC_FRAME_FORMAT_YUYV", UVC_FRAME_FORMAT_YUYV)
        .value("UVC_FRAME_FORMAT_UYVY", UVC_FRAME_FORMAT_UYVY)
        // 24-bit RGB
        .value("UVC_FRAME_FORMAT_RGB", UVC_FRAME_FORMAT_RGB)
        .value("UVC_FRAME_FORMAT_BGR", UVC_FRAME_FORMAT_BGR)
        // Motion-JPEG (or JPEG) encoded images
        .value("UVC_FRAME_FORMAT_MJPEG", UVC_FRAME_FORMAT_MJPEG)
        .value("UVC_FRAME_FORMAT_H264", UVC_FRAME_FORMAT_H264)
        // Greyscale images
        .value("UVC_FRAME_FORMAT_GRAY16", UVC_FRAME_FORMAT_GRAY16)
        .value("UVC_FRAME_FORMAT_GRAY8", UVC_FRAME_FORMAT_GRAY8)
        // Raw colour mosaic images
        .value("UVC_FRAME_FORMAT_BY8", UVC_FRAME_FORMAT_BY8)
        .value("UVC_FRAME_FORMAT_BA81", UVC_FRAME_FORMAT_BA81)
        .value("UVC_FRAME_FORMAT_SGRBG8", UVC_FRAME_FORMAT_SGRBG8)
        .value("UVC_FRAME_FORMAT_SGBRG8", UVC_FRAME_FORMAT_SGBRG8)
        .value("UVC_FRAME_FORMAT_SRGGB8", UVC_FRAME_FORMAT_SRGGB8)
        .value("UVC_FRAME_FORMAT_SBGGR8", UVC_FRAME_FORMAT_SBGGR8)
        // YUV420: NV12
        .value("UVC_FRAME_FORMAT_NV12", UVC_FRAME_FORMAT_NV12)
        // Number of formats understood
        .value("UVC_FRAME_FORMAT_COUNT", UVC_FRAME_FORMAT_COUNT)
        .export_values()
    ;

    py::register_exception<WebcamException>(m, "WebcamException");

    py::class_<DeviceDescriptor, std::shared_ptr<DeviceDescriptor>>(m, "DeviceDescriptor")
        .def_readonly("idVendor", &DeviceDescriptor::idVendor)
        .def_readonly("idProduct", &DeviceDescriptor::idProduct)
        .def_readonly("bcdUVC", &DeviceDescriptor::bcdUVC)
        .def_readonly("serialNumber", &DeviceDescriptor::serialNumber)
        .def_readonly("manufacturer", &DeviceDescriptor::manufacturer)
        .def_readonly("product", &DeviceDescriptor::product)
        .def("__repr__", &DeviceDescriptor::to_string)
    ;

    py::class_<WebcamDataRaw, Message, std::shared_ptr<WebcamDataRaw>>(m, "WebcamDataRaw")
        .def(py::init([](const std::shared_ptr<core::Message> o) {
            return std::make_shared<WebcamDataRaw>(*o); }),
            "Construct a WebcamDataRaw from a core message",
            py::arg("other"))
        .def_property_readonly("t0", &WebcamDataRaw::get_t0)
        .def_property_readonly("t1", &WebcamDataRaw::get_t1)
        .def_property_readonly("width", &WebcamDataRaw::get_width)
        .def_property_readonly("height", &WebcamDataRaw::get_height)
        .def_property_readonly("data", &WebcamDataRaw::get_data)
        .def_property_readonly("data_size_bytes", &WebcamDataRaw::get_data_size_bytes)
        .def_property_readonly("uvc_frame_format", &WebcamDataRaw::get_uvc_frame_format)
        .def_property_readonly("sequence", &WebcamDataRaw::get_sequence)
        .def_property_readonly("capture_time", &WebcamDataRaw::get_capture_time)
        .def("__repr__", &WebcamDataRaw::to_string)
    ;

    py::class_<WebcamDataRGB, Message, std::shared_ptr<WebcamDataRGB>>(m, "WebcamDataRGB")
        .def(py::init([](const std::shared_ptr<core::Message> o) {
            return std::make_shared<WebcamDataRGB>(*o); }),
            "Construct a WebcamDataRGB from a core message",
            py::arg("other"))
        .def_property_readonly("t0", &WebcamDataRGB::get_t0)
        .def_property_readonly("t1", &WebcamDataRGB::get_t1)
        .def_property_readonly("rgb", &WebcamDataRGB::get_rgb)
        .def_property_readonly("sequence", &WebcamDataRGB::get_sequence)
        .def_property_readonly("capture_time", &WebcamDataRGB::get_capture_time)
        .def("__repr__", &WebcamDataRGB::to_string)
    ;

    py::class_<WebcamSensor, RunnableNode, std::shared_ptr<WebcamSensor>>(m, "WebcamSensor")
        .def(
            py::init<uint16_t, uint16_t, uint16_t, int8_t, uvc_frame_format, bool, const std::string&>(),
            "Create a Webcam sensor",
            py::arg("width"),
            py::arg("height"),
            py::arg("fps"),
            py::arg("device_index") = -1,
            py::arg("format") = UVC_FRAME_FORMAT_ANY,
            py::arg("emit_rgb") = true,
            py::arg("name") = "WebcamSensor")
        .def("print_device_info", &WebcamSensor::print_device_info)
    ;

    py::class_<WebcamRawToRGBConverter, Node, std::shared_ptr<WebcamRawToRGBConverter>>(m, "WebcamRawToRGBConverter")
        .def(
            py::init<const std::string&>(),
            "Create a WebcamRawToRGBConverter",
            py::arg("name") = "WebcamRawToRGBConverter")
    ;
}
