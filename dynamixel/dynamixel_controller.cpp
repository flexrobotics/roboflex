#include <iostream>
#include <sstream>
#include <iomanip>
#include <numeric>
#include <type_traits>
#include "dynamixel/dynamixel_controller.h"
#include "core/util/utils.h"

using std::stringstream;


// Some utility functions ...


namespace roboflex {
namespace dynamixelgroup {


// Some more utility functions...

int sum_size_control_table_entries(const vector<DXLControlTable> &v) {
    vector<int> sizes;
    std::transform(v.begin(), v.end(), std::back_inserter(sizes),
        [](const DXLControlTable &c) {
            return ControlTableEntriesToSizes.find(c)->second;
        });
    return vector_sum(sizes);
}

int get_highest_total_size(const DXLIdsToControlTableEntries &dxl_ids_to_values) {
    vector<int> total_sizes;
    std::transform(dxl_ids_to_values.begin(), dxl_ids_to_values.end(), std::back_inserter(total_sizes),
        [](const std::pair<const int, vector<DXLControlTable>> & p) {
            return sum_size_control_table_entries(p.second);
        });
    return *std::max_element(total_sizes.begin(), total_sizes.end());
}

std::string control_table_entry_map_to_string(const DXLIdsToControlTableEntries& dxls_ids_to_values)
{
    stringstream sst;
    sst << "{";
    for (auto p: dxls_ids_to_values) {
        sst << p.first << ": [";
        for (auto c: p.second) {
            sst << to_underlying(c) << "(" << ControlTableEntriesToNames.at(c) << "), ";
        }
        sst << "], ";
    }
    sst << "}";
    return sst.str();
}


// -- DynamixelGroupState -- 

void DynamixelGroupState::print_on(std::ostream& os) const
{
    os << this->values;
    os << std::fixed << std::setprecision(3)
       << " t0: "  << this->timestamp.t0 
       << " t1: " << this->timestamp.t1;
}

string DynamixelGroupState::to_string() const {
    stringstream sst;
    sst << "<DynamixelGroupState ";
    this->print_on(sst);
    sst << ">";
    return sst.str();
}


// -- DynamixelGroupCommand -- 

void DynamixelGroupCommand::print_on(std::ostream& os) const
{
    os << this->values << " should_write: " << this->should_write;
    os << std::fixed << std::setprecision(3)
       << " t0: "  << this->timestamp.t0 
       << " t1: " << this->timestamp.t1;
}

string DynamixelGroupCommand::to_string() const {
    stringstream sst;
    sst << "<DynamixelGroupCommand ";
    this->print_on(sst);
    sst << ">";
    return sst.str();
}


// -- DynamixelGroupController --

DynamixelGroupController::DynamixelGroupController(
    const string &device_name,
    int baud_rate):
        device_name(device_name),
        baud_rate(baud_rate),
        port_handler(dynamixel::PortHandler::getPortHandler(device_name.c_str())),
        packet_handler(dynamixel::PacketHandler::getPacketHandler(2.0)),
        sync_reader(nullptr),
        sync_writer(nullptr)
{
    if (!port_handler->openPort()) {
        throw DynamixelException("DynamixelGroup constructor: invalid device name");
    }

    if (!port_handler->setBaudRate(baud_rate)) {
        port_handler->closePort();
        throw DynamixelException("DynamixelGroup constructor: unable to set baud rate");
    }
}

DynamixelGroupController::DynamixelGroupController(
    const string& device_name,
    int baud_rate,
    const vector<DXLId>& dxl_ids,
    const vector<OperatingMode>& operating_modes,
    const DXLIdsToControlTableEntries& read_control_map,
    const DXLIdsToControlTableEntries& write_control_map):
        DynamixelGroupController(device_name, baud_rate)
{
    set_operating_modes(dxl_ids, operating_modes);
    set_sync_read(read_control_map);
    set_sync_write(write_control_map);
    enable_torque(dxl_ids);
}

/**
 * Each key in the map is mapped to a copy of the same value.
 */
template <typename MapType, class Keytype, typename AllType>
MapType map_each_to_all(
    const Keytype& keys, 
    const AllType& value)
{
    MapType result;
    for (auto key: keys) {
        result[key] = value;
    }
    return result;
}

DynamixelGroupController::DynamixelGroupController(
    const string& device_name,
    int baud_rate,
    const vector<DXLId>& dxl_ids,
    const OperatingMode operating_mode,
    const vector<DXLControlTable>& read_control_list,
    const vector<DXLControlTable>& write_control_list):
        DynamixelGroupController(
            device_name, 
            baud_rate,
            dxl_ids,
            vector<OperatingMode>(dxl_ids.size(), operating_mode),
            map_each_to_all<DXLIdsToControlTableEntries>(dxl_ids, read_control_list),
            map_each_to_all<DXLIdsToControlTableEntries>(dxl_ids, write_control_list))
{

} 

DynamixelGroupController::~DynamixelGroupController() 
{
    delete sync_reader;
    delete sync_writer;
    port_handler->closePort();
    delete port_handler;
    //delete packet_handler; // NOPE, IT'S A SINGLETON
}

void DynamixelGroupController::check_write_txrx(const string & name, DXLId dxl_id, int value, int control_table_index, int result, uint8_t dxl_error) {
    if (result != COMM_SUCCESS) {
        stringstream es;
        es << "DynamixelGroupController failure in " << name << " (" << dxl_id << ", "
            << control_table_index << ", " << value << ") -> " << result << ": "
            << packet_handler->getTxRxResult(result);
        throw DynamixelException(es.str());
    } else if (dxl_error != 0) {
        stringstream es;
        es << "DynamixelGroupController error in " << name << " (" << dxl_id << ", "
            << control_table_index << ", " << value << ") -> " << result << ": "
            << dxl_error << " " << packet_handler->getRxPacketError(dxl_error);
        throw DynamixelException(es.str());
    }
}

void DynamixelGroupController::check_read_txrx(const string & name, DXLId dxl_id, int control_table_index, int result, uint8_t dxl_error) {
    if (result != COMM_SUCCESS) {
        stringstream es;
        es << "DynamixelGroupController failure in " << name << " (" << dxl_id << ", "
            << control_table_index << ") -> " << result << ": "
            << packet_handler->getTxRxResult(result);
        throw DynamixelException(es.str());
    } else if (dxl_error != 0) {
        stringstream es;
        es << "DynamixelGroupController error in " << name << " (" << dxl_id << ", "
            << control_table_index << ") -> " << result << ": "
            << dxl_error << " " << packet_handler->getRxPacketError(dxl_error);
        throw DynamixelException(es.str());
    }
}

void DynamixelGroupController::write_1byte_txrx(DXLId dxl_id, int control_table_index, int value) {
    uint8_t dxl_error = 0;
    int result = packet_handler->write1ByteTxRx(port_handler, dxl_id, control_table_index, value, &dxl_error);
    check_write_txrx("write_1byte_txrx", dxl_id, value, control_table_index, result, dxl_error);
}

void DynamixelGroupController::write_2byte_txrx(DXLId dxl_id, int control_table_index, int value) {
    uint8_t dxl_error = 0;
    int result = packet_handler->write2ByteTxRx(port_handler, dxl_id, control_table_index, value, &dxl_error);
    check_write_txrx("write_2byte_txrx", dxl_id, value, control_table_index, result, dxl_error);
}

void DynamixelGroupController::write_4byte_txrx(DXLId dxl_id, int control_table_index, int value) {
    uint8_t dxl_error = 0;
    int result = packet_handler->write4ByteTxRx(port_handler, dxl_id, control_table_index, value, &dxl_error);
    check_write_txrx("write_4byte_txrx", dxl_id, value, control_table_index, result, dxl_error);
}

int DynamixelGroupController::read_1byte_txrx(DXLId dxl_id, int control_table_index) {
    uint8_t dxl_error = 0;
    uint8_t data = 0;
    int result = packet_handler->read1ByteTxRx(port_handler, dxl_id, control_table_index, &data, &dxl_error);
    check_read_txrx("read_1byte_txrx", dxl_id, control_table_index, result, dxl_error);
    return data;
}

int DynamixelGroupController::read_2byte_txrx(DXLId dxl_id, int control_table_index) {
    uint8_t dxl_error = 0;
    uint16_t data = 0;
    int result = packet_handler->read2ByteTxRx(port_handler, dxl_id, control_table_index, &data, &dxl_error);
    check_read_txrx("read_2byte_txrx", dxl_id, control_table_index, result, dxl_error);
    return data;
}

int DynamixelGroupController::read_4byte_txrx(DXLId dxl_id, int control_table_index) {
    uint8_t dxl_error = 0;
    uint32_t data = 0;
    int result = packet_handler->read4ByteTxRx(port_handler, dxl_id, control_table_index, &data, &dxl_error);
    check_read_txrx("read_4byte_txrx", dxl_id, control_table_index, result, dxl_error);
    return data;
}

bool DynamixelGroupController::get_torque_enabled(DXLId dxl_id) {
    return read_1byte_txrx(dxl_id, to_underlying(DXLControlTable::OperatingMode));
}

void DynamixelGroupController::enable_torque(const vector<DXLId> & dxl_ids) {
    for (DXLId dxl_id: dxl_ids) {
        write_1byte_txrx(dxl_id, to_underlying(DXLControlTable::TorqueEnable), 1);
    }
}

void DynamixelGroupController::disable_torque(const vector<DXLId> & dxl_ids) {
    for (DXLId dxl_id: dxl_ids) {
        write_1byte_txrx(dxl_id, to_underlying(DXLControlTable::TorqueEnable), 0);
    }
}

void DynamixelGroupController::write_directly(DXLId dxl_id, DXLControlTable control_table_entry, int value, bool disable_torque_) 
{
    bool torque_was_enabled = false;
    if (disable_torque_) {
        torque_was_enabled = get_torque_enabled(dxl_id);
        if (!torque_was_enabled) {
            disable_torque({ dxl_id });
        }
    }

    int v = to_underlying(control_table_entry);
    int size = ControlTableEntriesToSizes.find(control_table_entry)->second;
    switch (size) {
        case 1:
            write_1byte_txrx(dxl_id, v, value);
            break;

        case 2:
            write_2byte_txrx(dxl_id, v, value);
            break;

        case 4:
            write_4byte_txrx(dxl_id, v, value);
            break;
    }

    if (disable_torque_ && torque_was_enabled) {
        enable_torque({ dxl_id });
    }
}

void DynamixelGroupController::set_operating_mode(DXLId dxl_id, OperatingMode operating_mode) {
    bool torque_enabled = get_torque_enabled(dxl_id);
    if (torque_enabled) {
        disable_torque({ dxl_id });
    }
    write_1byte_txrx(dxl_id, to_underlying(DXLControlTable::OperatingMode), to_underlying(operating_mode));
    if (torque_enabled) {
        enable_torque({ dxl_id });
    }
}

void DynamixelGroupController::set_operating_modes(const vector<DXLId> & dxl_ids, OperatingMode operating_mode) {
    for (DXLId dxl_id: dxl_ids) {
        set_operating_mode(dxl_id, operating_mode);
    }
}

void DynamixelGroupController::set_operating_modes(const vector<DXLId> & dxl_ids, const vector<OperatingMode> & operating_modes) {
    for (size_t i=0; i<dxl_ids.size(); i++) {
        set_operating_mode(dxl_ids[i], operating_modes[i]);
    }
}

void DynamixelGroupController::set_current_control(const vector<DXLId> & dxl_ids) {
    set_operating_modes(dxl_ids, OperatingMode::CurrentControl);
}

void DynamixelGroupController::set_velocity_control(const vector<DXLId> & dxl_ids) {
    set_operating_modes(dxl_ids, OperatingMode::VelocityControl);
}

void DynamixelGroupController::set_position_control(const vector<DXLId> & dxl_ids) {
    set_operating_modes(dxl_ids, OperatingMode::PositionControl);
}

void DynamixelGroupController::set_extended_position_control(const vector<DXLId> & dxl_ids) {
    set_operating_modes(dxl_ids, OperatingMode::ExtendedPositionControl);
}

void DynamixelGroupController::set_position_current_control(const vector<DXLId> & dxl_ids) {
    set_operating_modes(dxl_ids, OperatingMode::PositionCurrentControl);
}

void DynamixelGroupController::set_pwm_control(const vector<DXLId> & dxl_ids) {
    set_operating_modes(dxl_ids, OperatingMode::PWMControl);
}

int DynamixelGroupController::get_operating_mode(DXLId dxl_id) {
    return read_1byte_txrx(dxl_id, to_underlying(DXLControlTable::OperatingMode));
}

vector<uint8_t> DynamixelGroupController::ping() {
    vector<uint8_t> vec;
    int dxl_comm_result = packet_handler->broadcastPing(port_handler, vec);
    if (dxl_comm_result != COMM_SUCCESS) {
        throw DynamixelException(string("broadcastPing failed: ") + packet_handler->getTxRxResult(dxl_comm_result));
    }
    return vec;
}

void DynamixelGroupController::set_sync_read(const DXLIdsToControlTableEntries &dxl_ids_to_values) 
{
    delete sync_reader;

    sync_read_settings = dxl_ids_to_values;

    int max_size = get_highest_total_size(dxl_ids_to_values);
    sync_reader = new dynamixel::GroupSyncRead(
        port_handler,
        packet_handler,
        IndirectDataForReading,
        max_size);

    for (DXLIdsToControlTableEntries::value_type element: dxl_ids_to_values) {
        DXLId dxl_id = element.first;
        auto values_to_set = element.second;

        int i = 0;
        for (DXLControlTable v: values_to_set) {
            int size = ControlTableEntriesToSizes.find(v)->second;

            for (int j=0; j<size; j++) {
                uint16_t indirect_address = IndirectAddressForReading + i*2;
                uint16_t target_address = to_underlying(v) + j;
                write_2byte_txrx(dxl_id, indirect_address, target_address);
                i++;
            }
        }

        bool result = sync_reader->addParam(dxl_id);
        if (!result) {
            string e = "DynamixelGroupController::set_sync_read failed adding param " +
                std::to_string(dxl_id) +  " to sync_reader";
            throw DynamixelException(e);
        }
    }
}

void DynamixelGroupController::set_sync_write(const DXLIdsToControlTableEntries &dxl_ids_to_values) 
{
    delete sync_writer;

    sync_write_settings = dxl_ids_to_values;

    int max_size = get_highest_total_size(dxl_ids_to_values);
    sync_writer = new dynamixel::GroupSyncWrite(
        port_handler,
        packet_handler,
        IndirectDataForWriting,
        max_size);

    for (DXLIdsToControlTableEntries::value_type element: dxl_ids_to_values) {
        DXLId dxl_id = element.first;
        auto values_to_set = element.second;

        int i = 0;
        for (DXLControlTable v: values_to_set) {
            int size = ControlTableEntriesToSizes.find(v)->second;

            for (int j=0; j<size; j++) {
                uint16_t indirect_address = IndirectAddressForWriting + i*2;
                uint16_t target_address = to_underlying(v) + j;
                write_2byte_txrx(dxl_id, indirect_address, target_address);
                i++;
            }
        }
    }
}

DynamixelGroupState DynamixelGroupController::read() 
{
    // Perform the actual read
    double t0 = core::get_current_time();
    int result = sync_reader->txRxPacket();
    double t1 = core::get_current_time();

    if (result != COMM_SUCCESS) {
        throw DynamixelException("DynamixelGroupController::process_thread_fn sync read failed!");
    }

    DynamixelGroupState read_state {{}, {t0, t1}};

    for (DXLIdsToControlTableEntries::value_type element: sync_read_settings) {
        DXLId dxl_id = element.first;
        auto values_to_read = element.second;

        int offset = 0;
        for (DXLControlTable v: values_to_read) {

            // details about how to read from dynamixel control table...
            int size = ControlTableEntriesToSizes.find(v)->second;
            int read_position = IndirectDataForReading + offset;

            // make sure data is available to read
            bool data_available = sync_reader->isAvailable(dxl_id, read_position, size);
            if (!data_available) {
                stringstream es;
                es << "DynamixelGroupController::process_thread_fn in sync read: data not available"
                   << " for dxl_id: " << dxl_id << " table entry: " << to_underlying(v);
                throw DynamixelException(es.str());
            }

            // perform the actual read
            int32_t value = sync_reader->getData(dxl_id, read_position, size);

            // insert into the state
            read_state.values[dxl_id][v] = value;

            offset += size;
        }
    }

    return read_state;
}

void DynamixelGroupController::write(DynamixelGroupCommand &values_to_write) 
{
    for (DXLIdsToValues::value_type element: values_to_write.values) {
        DXLId dxl_id = element.first;
        auto values_to_set = element.second;

        // allocate a buffer to write data to
        int buffer_size = sum_size_control_table_entries(sync_write_settings[dxl_id]);
        auto buffer = vector<uint8_t>(buffer_size, 0);

        const DeviceValues &write_values = values_to_write.values.at(dxl_id);
        const vector<DXLControlTable> &control_table_entries = sync_write_settings.at(dxl_id);

        int i = 0;

        for (DXLControlTable control_table_entry: control_table_entries) {
            int value_to_write = write_values.at(control_table_entry);
            int entry_size = ControlTableEntriesToSizes.at(control_table_entry);

            switch (entry_size) {
                case 1:
                    buffer[i+0] = DXL_LOBYTE(DXL_LOWORD(value_to_write));
                    i += 1;
                    break;

                case 2:
                    buffer[i+0] = DXL_LOBYTE(DXL_LOWORD(value_to_write));
                    buffer[i+1] = DXL_HIBYTE(DXL_LOWORD(value_to_write));
                    i += 2;
                    break;

                case 4:
                    buffer[i+0] = DXL_LOBYTE(DXL_LOWORD(value_to_write));
                    buffer[i+1] = DXL_HIBYTE(DXL_LOWORD(value_to_write));
                    buffer[i+2] = DXL_LOBYTE(DXL_HIWORD(value_to_write));
                    buffer[i+3] = DXL_HIBYTE(DXL_HIWORD(value_to_write));
                    i += 4;
                    break;
            }
        }

        bool result = sync_writer->addParam(dxl_id, buffer.data());
        if (!result) {
            throw DynamixelException("DynamixelGroupController::write_values, failed to add param to " + std::to_string(dxl_id));
        }
    }

    // perform the actual write to device
    double t0 = core::get_current_time();
    int result = sync_writer->txPacket();
    double t1 = core::get_current_time();

    if (result != COMM_SUCCESS) {
        string e = string("DynamixelGroupController::write_values, transaction failed: ") + packet_handler->getTxRxResult(result) + " Result Code: " + std::to_string(result);
        throw DynamixelException(e);
    }

    // must be done after each write (unless, I suppose we want to write the exact same thing again)
    sync_writer->clearParam();

    // The write timestamps will be written to the
    // command we were sent.
    values_to_write.timestamp = {t0, t1};
}

bool DynamixelGroupController::reboot(DXLId dxl_id)
{
    uint8_t dxl_error = 0;
    int dxl_comm_result = packet_handler->reboot(port_handler, dxl_id, &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS) {
        std::cerr << "DynamixelGroup::reboot(" << dxl_id << ") FAILED, NO SUCCESS: " << dxl_comm_result << " " << packet_handler->getTxRxResult(dxl_comm_result) << std::endl << std::flush;
    } else if (dxl_error != 0) {
        std::cerr << "DynamixelGroup::reboot(" << dxl_id << ") FAILED, NONZERO ERROR: " << dxl_error << " " << packet_handler->getRxPacketError(dxl_error) << std::endl << std::flush;
    } else {
        //std::cout << "DynamixelGroup::reboot apparently succeeded: " << dxl_comm_result << std::endl << std::flush;
    }

    bool rebooted = dxl_comm_result == COMM_SUCCESS;
    return rebooted;
}

void DynamixelGroupController::run_readwrite_loop(ReadWriteLoopFunction f)
{
    bool should_continue = true;
    DynamixelGroupCommand command;
    while (should_continue) {
        auto state = this->read();
        should_continue = f(state, command);
        if (should_continue &&  command.should_write) {
            write(command);
        }
    }
}

} // namespace dynamixelgroup
} // namespace roboflex
