#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "core/core.h"
#include "dynamixel/dynamixel.h"
#include "dynamixel/dynamixel_controller.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::dynamixelgroup;
using namespace roboflex::dynamixelnodes;

PYBIND11_MODULE(roboflex_dynamixel_ext, m) {
    m.doc() = "roboflex_dynamixel_ext";

    py::register_exception<DynamixelException>(m, "DynamixelException");

    py::class_<TimestampPair>(m, "TimestampPair")
        .def_readonly("t0", &TimestampPair::t0)
        .def_readonly("t1", &TimestampPair::t1)
    ;

    py::class_<DynamixelGroupState>(m, "DynamixelGroupState")
        .def_readonly("values", &DynamixelGroupState::values)
        .def_readonly("timestamp", &DynamixelGroupState::timestamp)
        .def("__repr__", &DynamixelGroupState::to_string)
    ;

    py::class_<DynamixelGroupCommand>(m, "DynamixelGroupCommand")
        .def_readonly("values", &DynamixelGroupCommand::values)
        .def_readonly("timestamp", &DynamixelGroupCommand::timestamp)
        .def("set", [](DynamixelGroupCommand& s, DXLId dxl_id, DXLControlTable key, int value){
            s.values[dxl_id][key] = value;
        })
        .def("__repr__", &DynamixelGroupCommand::to_string)
    ;

    py::class_<DynamixelGroupStateMessage, core::Message, std::shared_ptr<DynamixelGroupStateMessage>>(m, "DynamixelGroupStateMessage")
        .def(py::init<const DynamixelGroupState&>(),
            "Create a DynamixelGroupStateMessage.",
            py::arg("state"))
        .def(py::init([](const std::shared_ptr<core::Message> o) {
            return std::make_shared<DynamixelGroupStateMessage>(*o); }),
            "Construct a DynamixelGroupStateMessage from a core message",
            py::arg("other"))
        .def_property_readonly("state", &DynamixelGroupStateMessage::get_state)
        .def("__repr__", &DynamixelGroupStateMessage::to_string)
    ;

    py::class_<DynamixelGroupCommandMessage, core::Message, std::shared_ptr<DynamixelGroupCommandMessage>>(m, "DynamixelGroupCommandMessage")
        .def(py::init<const DynamixelGroupCommand&>(),
            "Create a DynamixelGroupCommandMessage.",
            py::arg("command"))
        .def(py::init([](const std::shared_ptr<core::Message> o) {
            return std::make_shared<DynamixelGroupCommandMessage>(*o); }),
            "Construct a DynamixelGroupCommandMessage from a core message",
            py::arg("other"))
        .def_property_readonly("command", &DynamixelGroupCommandMessage::get_command)
        .def("__repr__", &DynamixelGroupCommandMessage::to_string)
    ;

    py::enum_<DXLControlTable>(m, "DXLControlTable")
        .value("OperatingMode"       , DXLControlTable::OperatingMode)

        .value("TemperatureLimit"    , DXLControlTable::TemperatureLimit)
        .value("MaxVoltage"          , DXLControlTable::MaxVoltage)
        .value("MinVoltage"          , DXLControlTable::MinVoltage)
        .value("PWMLimit"            , DXLControlTable::PWMLimit)
        .value("MaxCurrent"          , DXLControlTable::MaxCurrent)
        .value("MaxVelocity"         , DXLControlTable::MaxVelocity)
        .value("MaxPosition"         , DXLControlTable::MaxPosition)
        .value("MinPosition"         , DXLControlTable::MinPosition)

        .value("TorqueEnable"        , DXLControlTable::TorqueEnable)
        .value("LED"                 , DXLControlTable::LED)
        .value("StatusReturnLevel"   , DXLControlTable::StatusReturnLevel)
        .value("VelocityIGain"       , DXLControlTable::VelocityIGain)
        .value("VelocityPGain"       , DXLControlTable::VelocityPGain)
        .value("PositionDGain"       , DXLControlTable::PositionDGain)
        .value("PositionIGain"       , DXLControlTable::PositionIGain)
        .value("PositionPGain"       , DXLControlTable::PositionPGain)
        .value("Feedforward2Gain"    , DXLControlTable::Feedforward2Gain)
        .value("Feedforward1Gain"    , DXLControlTable::Feedforward1Gain)
        .value("BusWatchdog"         , DXLControlTable::BusWatchdog)
        .value("GoalPWM"             , DXLControlTable::GoalPWM)
        .value("GoalCurrent"         , DXLControlTable::GoalCurrent)
        .value("GoalVelocity"        , DXLControlTable::GoalVelocity)
        .value("ProfileAcceleration" , DXLControlTable::ProfileAcceleration)
        .value("ProfileVelocity"     , DXLControlTable::ProfileVelocity)
        .value("GoalPosition"        , DXLControlTable::GoalPosition)
        .value("RealtimeTick"        , DXLControlTable::RealtimeTick)
        .value("Moving"              , DXLControlTable::Moving)
        .value("MovingStatus"        , DXLControlTable::MovingStatus)
        .value("PresentPWM"          , DXLControlTable::PresentPWM)
        .value("PresentCurrent"      , DXLControlTable::PresentCurrent)
        .value("PresentVelocity"     , DXLControlTable::PresentVelocity)
        .value("PresentPosition"     , DXLControlTable::PresentPosition)
        .value("VelocityTrajectory"  , DXLControlTable::VelocityTrajectory)
        .value("PresentTrajectory"   , DXLControlTable::PresentTrajectory)
        .value("PresentInputVoltage" , DXLControlTable::PresentInputVoltage)
        .value("PresentTemperature"  , DXLControlTable::PresentTemperature)
    ;

    py::class_<DynamixelGroupController, std::shared_ptr<DynamixelGroupController>>(m, "DynamixelGroupController")
         .def(py::init<const std::string&, int>(),
            "Create a Dynamixel group controller.",
            py::arg("device_name"),
            py::arg("baud_rate"))
        .def("get_device_name", &DynamixelGroupController::get_device_name)
        .def("get_baud_rate", &DynamixelGroupController::get_baud_rate)
        .def("get_dynamixel_ids", &DynamixelGroupController::ping)
        .def("enable_torque", &DynamixelGroupController::enable_torque)
        .def("disable_torque", &DynamixelGroupController::disable_torque)
        .def("get_torque_enabled", &DynamixelGroupController::get_torque_enabled)
        .def("set_current_control", &DynamixelGroupController::set_current_control)
        .def("set_velocity_control", &DynamixelGroupController::set_velocity_control)
        .def("set_position_control", &DynamixelGroupController::set_position_control)
        .def("set_extended_position_control", &DynamixelGroupController::set_extended_position_control)
        .def("set_position_current_control", &DynamixelGroupController::set_position_current_control)
        .def("set_pwm_control", &DynamixelGroupController::set_pwm_control)
        .def("get_operating_mode", &DynamixelGroupController::get_operating_mode)
        .def("set_sync_read", &DynamixelGroupController::set_sync_read)
        .def("set_sync_write", &DynamixelGroupController::set_sync_write)
        .def("read", &DynamixelGroupController::read)
        .def("write", &DynamixelGroupController::write)
        .def("reboot", &DynamixelGroupController::reboot)
        .def("ping", &DynamixelGroupController::ping)
        .def("run_readwrite_loop", [](
            DynamixelGroupController &d,
            pybind11::function rwf
        ) {
            auto rwf_local = [&rwf](
                const DynamixelGroupState& state,
                DynamixelGroupCommand& command)
            {
                auto resultobj = rwf(state, &command);
                bool retval = resultobj.cast<bool>();
                return retval;
            };
            d.run_readwrite_loop(rwf_local);
        },
            py::arg("rwf"))
     ;

    py::class_<DynamixelGroupNode, core::Node, std::shared_ptr<DynamixelGroupNode>>(m, "DynamixelGroupNode")
         .def(py::init<const std::string&, int, const std::string&>(),
            "Create a DynamixelGroupNode.",
            py::arg("device_name"),
            py::arg("baud_rate"),
            py::arg("name") = "DynamixelGroupNode")
        //.def("controller", &DynamixelGroupNode::controller)
    ;
}
