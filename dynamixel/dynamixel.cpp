#include "dynamixel.h"

namespace roboflex {
namespace dynamixelnodes {


// --- DynamixelGroupStateMessage ---

DynamixelGroupStateMessage::DynamixelGroupStateMessage(const DynamixelGroupState& state):
    core::Message(ModuleName, MessageName)
{
    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {

        // Write the state...
        for (DXLIdsToValues::value_type element: state.values) {
            DXLId dxl_id = element.first;
            string dxl_id_string = std::to_string(dxl_id);
            fbb.Key(dxl_id_string);
            fbb.Map([&]() {
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
        
        // Write the timestamp pair...
        fbb.Double("t0", state.timestamp.t0);
        fbb.Double("t1", state.timestamp.t1);
    });

    _state = get_state();
}

DynamixelGroupState DynamixelGroupStateMessage::get_state() const
{
    if (!_state_initialized) {

        // read the state values map
        DXLIdsToValues values;
        auto control_map = get_root_as_map()["state"].AsMap();
        auto control_map_keys = control_map.Keys();
        for (size_t i=0; i<control_map_keys.size(); i++) {
            string dxl_id_string = control_map_keys[i].AsString().str();
            DXLId dxl_id = std::stoi(dxl_id_string);
            auto dynamixel_control_map = control_map[dxl_id_string].AsMap();
            auto dynamixel_control_map_keys = dynamixel_control_map.Keys();
            for (size_t j=0; i<dynamixel_control_map_keys.size(); j++) {
                auto control_table_key = dynamixel_control_map_keys[i].AsUInt16();
                string control_table_key_string = std::to_string(control_table_key);
                DXLControlTable control_table_entry = DXLControlTable{control_table_key};
                int control_value = dynamixel_control_map[control_table_key_string].AsInt16();
                values[dxl_id][control_table_entry] = control_value;
            }
        }

        // read the timestamp pair
        double t0 = get_root_as_map()["t0"].AsDouble();
        double t1 = get_root_as_map()["t1"].AsDouble();

        // initialize the cached state
        _state = DynamixelGroupState{values, TimestampPair{t0, t1}};

        _state_initialized = true;
    }

    return _state;
}

void DynamixelGroupStateMessage::print_on(std::ostream& os) const
{
    DynamixelGroupState state = this->get_state();
    os << "<DynamixelGroupStateMessage";
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
        for (DXLIdsToValues::value_type element: command.values) {
            DXLId dxl_id = element.first;
            string dxl_id_string = std::to_string(dxl_id);
            fbb.Key(dxl_id_string);
            fbb.Map([&]() {
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

    _command = get_command();
}

DynamixelGroupCommand DynamixelGroupCommandMessage::get_command() const
{
    if (!_command_initialized) {

        // read the state values map
        DXLIdsToValues values;
        auto control_map = get_root_as_map()["state"].AsMap();
        auto control_map_keys = control_map.Keys();
        for (size_t i=0; i<control_map_keys.size(); i++) {
            string dxl_id_string = control_map_keys[i].AsString().str();
            DXLId dxl_id = std::stoi(dxl_id_string);
            auto dynamixel_control_map = control_map[dxl_id_string].AsMap();
            auto dynamixel_control_map_keys = dynamixel_control_map.Keys();
            for (size_t j=0; i<dynamixel_control_map_keys.size(); j++) {
                auto control_table_key = dynamixel_control_map_keys[i].AsUInt16();
                string control_table_key_string = std::to_string(control_table_key);
                DXLControlTable control_table_entry = DXLControlTable{control_table_key};
                int control_value = dynamixel_control_map[control_table_key_string].AsInt16();
                values[dxl_id][control_table_entry] = control_value;
            }
        }

        // initialize the cached state
        _command = DynamixelGroupCommand{values, TimestampPair{0.0, 0.0}};

        _command_initialized = true;
    }

    return _command;
}

void DynamixelGroupCommandMessage::print_on(std::ostream& os) const
{
    DynamixelGroupCommand command = this->get_command();
    os << "<DynamixelGroupCommandMessage";
    command.print_on(os);
    os << " ";
    core::Message::print_on(os);
    os << ">";
}


// --- DynamixelGroupNode ---

DynamixelGroupNode::DynamixelGroupNode(
    const string& device_name,
    int baud_rate,
    const string& name):
        core::RunnableNode(name),
        controller(device_name, baud_rate)
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
            if (last_command_message != nullptr) {
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
    this->controller.run_readwrite_loop(f);
}

} // dynamixelnodes
} // roboflex
