#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xadapt.hpp>
#include "core/core.h"
#include "realsense/realsense.h"

namespace py  = pybind11;
using namespace roboflex::realsense;

PYBIND11_MODULE(roboflex_realsense_ext, m) {

    py::enum_<RealsenseConfig::D400VisualPreset>(m, "D400VisualPreset")
        .value("CUSTOM", RealsenseConfig::D400VisualPreset::CUSTOM)
        .value("DEFAULT", RealsenseConfig::D400VisualPreset::DEFAULT)
        .value("HAND", RealsenseConfig::D400VisualPreset::HAND)
        .value("HIGH_ACCURACY", RealsenseConfig::D400VisualPreset::HIGH_ACCURACY)
        .value("HIGH_DENSITY", RealsenseConfig::D400VisualPreset::HIGH_DENSITY)
        .value("MEDIUM_DENSITY", RealsenseConfig::D400VisualPreset::MEDIUM_DENSITY)
    ;

    using TFP = RealsenseConfig::TemporalFilterParameters;
    const auto default_tfp = TFP{};
    py::class_<TFP>(m, "TemporalFilterParameters")
        .def(py::init<float, float, int>(),
            py::arg("alpha") = default_tfp.alpha,
            py::arg("delta") = default_tfp.delta,
            py::arg("persistence_control") = default_tfp.persistence_control)
        .def_readwrite("alpha", &TFP::alpha)
        .def_readwrite("delta", &TFP::delta)
        .def_readwrite("persistence_control", &TFP::persistence_control)
    ;

    py::enum_<CameraType>(m, "CameraType")
        .value("NONE", CameraType::NONE)
        .value("RGB", CameraType::RGB)
        .value("DEPTH", CameraType::DEPTH)
    ;

    using Cfg = RealsenseConfig;
    const auto default_cfg = Cfg{};
    py::class_<Cfg>(m, "Config")
        .def(
            py::init([](CameraType align_to,
                        bool prioritize_ae,
                        std::unordered_map<std::string, unsigned int>& rgb_settings,
                        std::unordered_map<std::string, unsigned int>& depth_settings,
                        RealsenseConfig::D400VisualPreset depth_visual_preset,
                        std::optional<TFP> temporal_filter_parameters,
                        std::optional<int> hole_filling_mode,
                        std::optional<int> decimation_filter) {
                return std::unique_ptr<Cfg>(new Cfg{
                    align_to,
                    prioritize_ae,
                    {
                        rgb_settings["fps"],
                        rgb_settings["width"],
                        rgb_settings["height"],
                    },
                    {
                        depth_settings["fps"],
                        depth_settings["width"],
                        depth_settings["height"],
                    },
                    depth_visual_preset,
                    temporal_filter_parameters,
                    hole_filling_mode,
                    decimation_filter,
                });
            }),
            py::arg("align_to")       = default_cfg.align_to,
            py::arg("prioritize_ae")  = default_cfg.prioritize_ae,
            py::arg("rgb_settings")   = std::unordered_map<std::string, unsigned int>({
                {"fps", default_cfg.rgb_settings.fps},
                {"width", default_cfg.rgb_settings.width},
                {"height", default_cfg.rgb_settings.height},
            }),
            py::arg("depth_settings") = std::unordered_map<std::string, unsigned int>({
                {"fps", default_cfg.depth_settings.fps},
                {"width", default_cfg.depth_settings.width},
                {"height", default_cfg.depth_settings.height},
            }),
            py::arg("depth_visual_preset")  = default_cfg.depth_visual_preset,
            py::arg("temporal_filter_parameters") = default_cfg.temporal_filter_parameters,
            py::arg("hole_filling_mode") = default_cfg.hole_filling_mode,
            py::arg("decimation_filter") = default_cfg.decimation_filter)
        .def_readwrite("align_to", &Cfg::align_to)
        .def_readwrite("prioritize_ae", &Cfg::prioritize_ae)
        .def_readwrite("rgb_settings", &Cfg::rgb_settings)
        .def_readwrite("depth_settings", &Cfg::depth_settings)
        .def_readwrite("depth_visual_preset", &Cfg::depth_visual_preset)
        .def_readwrite("temporal_filter_parameters", &Cfg::temporal_filter_parameters)
        .def_readwrite("hole_filling_mode", &Cfg::hole_filling_mode)
        .def_readwrite("decimation_filter", &Cfg::decimation_filter)
        .def("__repr__", &Cfg::to_string)
    ;

    py::class_<RealsenseFrameset, roboflex::core::Message, std::shared_ptr<RealsenseFrameset>>(m, "RealsenseFrameset")
        .def(py::init([](std::shared_ptr<roboflex::core::Message> o) {
            return std::make_shared<RealsenseFrameset>(*o); }),
            "Construct an RealsenseFrameset from a core message",
            py::arg("other"))
        .def("t0", &RealsenseFrameset::get_t0)
        .def("t1", &RealsenseFrameset::get_t1)

        // NOTE: get_rgb() and get_depth() return xt::xarray_adaptor, which
        // is great for c++, but python can't deal with - so we have to
        // convert to xtensors (which are containers).
        .def_property_readonly("rgb", [](std::shared_ptr<RealsenseFrameset> f){
            xt::xtensor<uint8_t, 3> r;
            r = f->get_rgb();
            return r;
        })
        .def_property_readonly("depth", [](std::shared_ptr<RealsenseFrameset> f){
            xt::xtensor<uint16_t, 2> r;
            r = f->get_depth();
            return r;
        })
        .def_property_readonly("aligned_to", &RealsenseFrameset::get_aligned_to)
        .def_property_readonly("serial_number", &RealsenseFrameset::get_serial_number)
        .def_property_readonly("camera_k_rgb", &RealsenseFrameset::get_camera_k_rgb)
        .def_property_readonly("camera_k_depth", &RealsenseFrameset::get_camera_k_depth)
        .def_property_readonly("frame_number", &RealsenseFrameset::get_frame_number)
        .def_property_readonly("timestamp", &RealsenseFrameset::get_timestamp)
        .def("__repr__", &RealsenseFrameset::to_string)
    ;

    py::class_<RealsenseSensor, roboflex::core::RunnableNode, std::shared_ptr<RealsenseSensor>>(m, "RealsenseSensor")
        .def(py::init<const std::string&, const RealsenseConfig&, const std::string&>(),
            "Create a Realsense sensor",
            py::arg("serial_number"),
            py::arg("config"),
            py::arg("name") = "RealsenseSensor")
        .def_static("get_connected_device_serial_numbers", &RealsenseSensor::get_connected_device_serial_numbers)
        .def_static("get_one_sensor", &RealsenseSensor::get_one_sensor)
        .def("produce", &RealsenseSensor::produce)
        .def("get_camera_k", &RealsenseSensor::get_camera_k)
        .def_property_readonly("depth_camera_k", &RealsenseSensor::get_depth_camera_k)
        .def_property_readonly("color_camera_k", &RealsenseSensor::get_color_camera_k)
        .def_property_readonly("width_pixels_color", &RealsenseSensor::get_width_pixels_color)
        .def_property_readonly("height_pixels_color", &RealsenseSensor::get_height_pixels_color)
        .def_property_readonly("width_pixels_depth", &RealsenseSensor::get_width_pixels_depth)
        .def_property_readonly("height_pixels_depth", &RealsenseSensor::get_height_pixels_depth)
        .def_property_readonly("fps_color", &RealsenseSensor::get_fps_color)
        .def_property_readonly("fps_depth", &RealsenseSensor::get_fps_depth)
        .def_property_readonly("config", &RealsenseSensor::get_config)
        .def_property_readonly("serial_number", &RealsenseSensor::get_serial_number)
    ;
}
