#include "dynamixel.h"

namespace roboflex {
namespace dynamixelnodes {


// --- DynamixelGroupStateMessage ---

DynamixelGroupStateMessage::DynamixelGroupStateMessage(const DynamixelGroupState& state):
    core::Message(ModuleName, MessageName)
{
    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {

        // Write the state values map.
        fbb.Map("state", [&]() {
            for (DXLIdsToValues::value_type element: state.values) {
                DXLId dxl_id = element.first;
                string dxl_id_string = std::to_string(dxl_id);
                fbb.Map(dxl_id_string.c_str(), [&]() {
                    DeviceValues control_table_values = element.second;
                    for (DeviceValues::value_type control_table_entry: control_table_values) {
                        DXLControlTable control_table_key = control_table_entry.first;
                        int control_table_key_int = static_cast<std::underlying_type_t<DXLControlTable>>(control_table_key);
                        string control_table_key_string = std::to_string(control_table_key_int);
                        int control_table_value = control_table_entry.second;
                        fbb.Int(control_table_key_string.c_str(), control_table_value);
                    }
                });
            }
        });
        
        // Write the timestamp pair.
        fbb.Double("t0", state.timestamp.t0);
        fbb.Double("t1", state.timestamp.t1);
    });

    _state = get_state();
}

DynamixelGroupState DynamixelGroupStateMessage::get_state() const
{
    if (!_state_initialized) {

        // Read the state values map.
        DXLIdsToValues values;
        auto control_map = root_map()["state"].AsMap();
        auto control_map_keys = control_map.Keys();
        for (size_t i=0; i<control_map_keys.size(); i++) {
            string dxl_id_string = control_map_keys[i].AsString().str();
            DXLId dxl_id = std::stoi(dxl_id_string);
            auto dynamixel_control_map = control_map[dxl_id_string].AsMap();
            auto dynamixel_control_map_keys = dynamixel_control_map.Keys();
            for (size_t j=0; j<dynamixel_control_map_keys.size(); j++) {
                auto control_table_key = dynamixel_control_map_keys[j].AsString().str();
                int control_table_key_int = std::stol(control_table_key);
                DXLControlTable control_table_entry = DXLControlTable{control_table_key_int};
                int control_value = dynamixel_control_map[control_table_key].AsInt16();
                values[dxl_id][control_table_entry] = control_value;
            }
        }

        // Read the timestamp pair.
        double t0 = root_map()["t0"].AsDouble();
        double t1 = root_map()["t1"].AsDouble();

        // Initialize the cached state.
        _state = DynamixelGroupState{values, TimestampPair{t0, t1}};

        _state_initialized = true;
    }

    return _state;
}

void DynamixelGroupStateMessage::print_on(std::ostream& os) const
{
    DynamixelGroupState state = this->get_state();
    os << "<DynamixelGroupStateMessage ";
    state.print_on(os);
    os << " ";
    core::Message::print_on(os);
    os << ">";
}


// --- DynamixelGroupCommandMessage ---

DynamixelGroupCommandMessage::DynamixelGroupCommandMessage(const DynamixelGroupCommand& command):
    core::Message(ModuleName, MessageName)
{
    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {
        
        // Write the command values map.
        fbb.Map("command", [&]() {
            for (DXLIdsToValues::value_type element: command.values) {
                DXLId dxl_id = element.first;
                string dxl_id_string = std::to_string(dxl_id);
                fbb.Map(dxl_id_string.c_str(), [&]() {
                    DeviceValues control_table_values = element.second;
                    for (DeviceValues::value_type control_table_entry: control_table_values) {
                        DXLControlTable control_table_key = control_table_entry.first;
                        int control_table_key_int = static_cast<std::underlying_type_t<DXLControlTable>>(control_table_key);
                        string control_table_key_string = std::to_string(control_table_key_int);
                        int control_table_value = control_table_entry.second;
                        fbb.Int(control_table_key_string.c_str(), control_table_value);
                    }
                });
            }
        });
    });

    _command = get_command();
}

DynamixelGroupCommand DynamixelGroupCommandMessage::get_command() const
{
    if (!_command_initialized) {

        // Read the command values map.
        DXLIdsToValues values;
        auto control_map = root_map()["command"].AsMap();
        auto control_map_keys = control_map.Keys();
        for (size_t i=0; i<control_map_keys.size(); i++) {
            string dxl_id_string = control_map_keys[i].AsString().str();
            DXLId dxl_id = std::stoi(dxl_id_string);
            auto dynamixel_control_map = control_map[dxl_id_string].AsMap();
            auto dynamixel_control_map_keys = dynamixel_control_map.Keys();
            for (size_t j=0; j<dynamixel_control_map_keys.size(); j++) {
                auto control_table_key = dynamixel_control_map_keys[j].AsString().str();
                int control_table_key_int = std::stol(control_table_key);
                DXLControlTable control_table_entry = DXLControlTable{control_table_key_int};
                int control_value = dynamixel_control_map[control_table_key].AsInt16();
                values[dxl_id][control_table_entry] = control_value;
            }
        }

        // Initialize the cached state.
        _command = DynamixelGroupCommand{values, TimestampPair{0.0, 0.0}};

        _command_initialized = true;
    }

    return _command;
}

void DynamixelGroupCommandMessage::print_on(std::ostream& os) const
{
    DynamixelGroupCommand command = this->get_command();
    os << "<DynamixelGroupCommandMessage ";
    command.print_on(os);
    os << " ";
    core::Message::print_on(os);
    os << ">";
}


// --- DynamixelGroupNode ---

DynamixelGroupNode::DynamixelGroupNode(
    DynamixelGroupController::Ptr controller,
    const string& name):
        core::RunnableNode(name),
        controller(controller)
{

}

void DynamixelGroupNode::receive(core::MessagePtr m)
{
    const std::lock_guard<std::recursive_mutex> lock(last_command_message_mutex);
    last_command_message = std::make_shared<DynamixelGroupCommandMessage>(*m);
}

bool DynamixelGroupNode::readwrite_loop_function(
    const DynamixelGroupState& state, 
    DynamixelGroupCommand& command)
{
    bool should_continue = !this->stop_requested();

    if (should_continue) {
        {
            // Get the last command message
            const std::lock_guard<std::recursive_mutex> lock(last_command_message_mutex);

            // Populate the command from the last command message
            if (last_command_message == nullptr) {
                command.should_write = false;
            } else {
                command.should_write = true;
                command.values = last_command_message->get_command().values;
            }
        }

        // Signal the state
        this->signal(std::make_shared<DynamixelGroupStateMessage>(state));
    }

    return should_continue;
}

void DynamixelGroupNode::child_thread_fn()
{
    auto f = [this](const DynamixelGroupState& state, DynamixelGroupCommand& command) {
        return this->readwrite_loop_function(state, command);
    };

    this->controller->run_readwrite_loop(f);
}


// --- DynamixelRemoteController ---

DynamixelRemoteController::DynamixelRemoteController(
    const string& name):
        core::Node(name)
{

}

void DynamixelRemoteController::receive(core::MessagePtr m)
{
    if (m->message_name() == DynamixelGroupStateMessage::MessageName) {
        auto state = DynamixelGroupStateMessage(*m).get_state();
        DXLIdsToValues commanded_values = this->readwrite_loop_function(state);
        DynamixelGroupCommand command = DynamixelGroupCommand{commanded_values};
        this->signal(std::make_shared<DynamixelGroupCommandMessage>(command));
    }
}


// --- DynamixelRemoteFrequencyController ---

DynamixelRemoteFrequencyController::DynamixelRemoteFrequencyController(
    const float frequency_hz,
    const string& name):
        nodes::FrequencyGenerator(frequency_hz, name)
{

}

void DynamixelRemoteFrequencyController::receive(core::MessagePtr m)
{
    if (m->message_name() == DynamixelGroupStateMessage::MessageName) {
        this->state = DynamixelGroupStateMessage(*m).get_state();
    }
}

void DynamixelRemoteFrequencyController::on_trigger(double wall_clock_time)
{
    if (this->state.timestamp.t0 != 0) {
        DXLIdsToValues commanded_values = this->readwrite_loop_function(this->state);
        DynamixelGroupCommand command = DynamixelGroupCommand{commanded_values};
        this->signal(std::make_shared<DynamixelGroupCommandMessage>(command));
    }
}

} // dynamixelnodes
} // roboflex
