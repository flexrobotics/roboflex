#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "core/core.h"
#include "audio_alsa/audio_alsa.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::core;
using namespace roboflex::audio_alsa;


PYBIND11_MODULE(roboflex_audio_alsa_ext, m) {
    m.doc() = "roboflex_audio_alsa_ext";

    py::class_<AudioData, Message, std::shared_ptr<AudioData>>(m, "AudioData")
        .def(py::init([](const std::shared_ptr<core::Message> o) {
            return std::make_shared<AudioData>(*o); }),
            "Construct a AudioData from a core message",
            py::arg("other"))
        .def_property_readonly("t0", &AudioData::get_t0)
        .def_property_readonly("t1", &AudioData::get_t1)
        .def_property_readonly("data", &AudioData::get_audio_data)
    ;

    py::class_<AudioData32, Message, std::shared_ptr<AudioData32>>(m, "AudioData32")
        .def(py::init([](const std::shared_ptr<core::Message> o) {
            return std::make_shared<AudioData32>(*o); }),
            "Construct a AudioData from a core message",
            py::arg("other"))
        .def_property_readonly("t0", &AudioData32::get_t0)
        .def_property_readonly("t1", &AudioData32::get_t1)
        .def_property_readonly("data", &AudioData32::get_audio_data)
    ;

    py::enum_<AudioSensor::BitDepth>(m, "BitDepth")
        //.value("S8", AudioSensor::BitDepth::S8)
        //.value("U8", AudioSensor::BitDepth::U8)
        .value("S16LE", AudioSensor::BitDepth::S16LE)
        //.value("U16LE", AudioSensor::BitDepth::U16LE)
        .value("S24LE", AudioSensor::BitDepth::S24LE)
        //.value("U24LE", AudioSensor::BitDepth::U24LE)
        .value("S32LE", AudioSensor::BitDepth::S32LE)
        //.value("U32LE", AudioSensor::BitDepth::U32LE)
        //.value("F32LE", AudioSensor::BitDepth::F32LE)
        //.value("F64LE", AudioSensor::BitDepth::F64LE)
        .value("S24_3LE", AudioSensor::BitDepth::S24_3LE)
    ;

    py::class_<AudioSensor, Node, std::shared_ptr<AudioSensor>>(m, "AudioSensor")
        .def(py::init<const std::string &, int, int, int, int, AudioSensor::BitDepth, const std::string &, bool>(),
            "Create an Audio sensor",
            py::arg("name") = "AudioSensor",
            py::arg("channels") = 8,
            py::arg("sampling_rate") = 48000,
            py::arg("capture_frames") = 512,
            py::arg("produce_frames") = 1024,
            py::arg("bit_depth") = AudioSensor::BitDepth::S16LE,
            py::arg("device_name") = "default",
            py::arg("debug") = false)
    ;
}

