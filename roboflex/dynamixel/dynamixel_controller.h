#ifndef ROBOFLEX_DYNAMIXEL_CONTROLLER__H
#define ROBOFLEX_DYNAMIXEL_CONTROLLER__H

#include <functional>
#include <memory>
#include "dynamixel_sdk.h"

using std::string, std::vector, std::map;


template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

template <typename T>
T vector_sum(const vector<T> &v) {
    return std::accumulate(v.begin(), v.end(), 0);
}


namespace roboflex {
namespace dynamixelgroup {

// These are operating modes andcontrol table values for the X-series
// Dynamixel: specifically the XH430-V350-R model
// Also confirmed to work with XH430-W350 and XM540-W270
// http://emanual.robotis.com/docs/en/dxl/x/xh430-v350/#control-table-of-eeprom-area

enum class OperatingMode: int {
    CurrentControl          = 0,
    VelocityControl         = 1,
    PositionControl         = 3,
    ExtendedPositionControl = 4,
    PositionCurrentControl  = 5,
    PWMControl              = 16
};

enum class DXLControlTable: int {
    OperatingMode       = 11,
    TemperatureLimit    = 31,
    MaxVoltage          = 32,
    MinVoltage          = 34,
    PWMLimit            = 36,
    MaxCurrent          = 38,
    MaxVelocity         = 44,
    MaxPosition         = 48,
    MinPosition         = 52,
    TorqueEnable        = 64,
    LED                 = 65,
    StatusReturnLevel   = 68,
    VelocityIGain       = 76,
    VelocityPGain       = 78,
    PositionDGain       = 80,
    PositionIGain       = 82,
    PositionPGain       = 84,
    Feedforward2Gain    = 88,
    Feedforward1Gain    = 90,
    BusWatchdog         = 98,
    GoalPWM             = 100,
    GoalCurrent         = 102,
    GoalVelocity        = 104,
    ProfileAcceleration = 108,
    ProfileVelocity     = 112,
    GoalPosition        = 116,
    RealtimeTick        = 120,
    Moving              = 122,
    MovingStatus        = 123,
    PresentPWM          = 124,
    PresentCurrent      = 126,
    PresentVelocity     = 128,
    PresentPosition     = 132,
    VelocityTrajectory  = 136,
    PresentTrajectory   = 140,
    PresentInputVoltage = 144,
    PresentTemperature  = 146,
};

typedef map<DXLControlTable, int> ControlTableSizeMap;
const ControlTableSizeMap ControlTableEntriesToSizes = {
    { DXLControlTable::TemperatureLimit    , 1 },
    { DXLControlTable::MaxVoltage          , 2 },
    { DXLControlTable::MinVoltage          , 2 },
    { DXLControlTable::PWMLimit            , 2 },
    { DXLControlTable::MaxCurrent          , 2 },
    { DXLControlTable::MaxVelocity         , 4 },
    { DXLControlTable::MaxPosition         , 4 },
    { DXLControlTable::MinPosition         , 4 },
    { DXLControlTable::TorqueEnable        , 1 },
    { DXLControlTable::LED                 , 1 },
    { DXLControlTable::StatusReturnLevel   , 1 },
    { DXLControlTable::VelocityIGain       , 2 },
    { DXLControlTable::VelocityPGain       , 2 },
    { DXLControlTable::PositionDGain       , 2 },
    { DXLControlTable::PositionIGain       , 2 },
    { DXLControlTable::PositionPGain       , 2 },
    { DXLControlTable::Feedforward2Gain    , 2 },
    { DXLControlTable::Feedforward1Gain    , 2 },
    { DXLControlTable::BusWatchdog         , 1 },
    { DXLControlTable::GoalPWM             , 2 },
    { DXLControlTable::GoalCurrent         , 2 },
    { DXLControlTable::GoalVelocity        , 4 },
    { DXLControlTable::ProfileAcceleration , 4 },
    { DXLControlTable::ProfileVelocity     , 4 },
    { DXLControlTable::GoalPosition        , 4 },
    { DXLControlTable::RealtimeTick        , 2 },
    { DXLControlTable::Moving              , 1 },
    { DXLControlTable::MovingStatus        , 1 },
    { DXLControlTable::PresentPWM          , 2 },
    { DXLControlTable::PresentCurrent      , 2 },
    { DXLControlTable::PresentVelocity     , 4 },
    { DXLControlTable::PresentPosition     , 4 },
    { DXLControlTable::VelocityTrajectory  , 4 },
    { DXLControlTable::PresentTrajectory   , 4 },
    { DXLControlTable::PresentInputVoltage , 2 },
    { DXLControlTable::PresentTemperature  , 1 }
};

typedef map<DXLControlTable, string> ControlTableNameMap;
const ControlTableNameMap ControlTableEntriesToNames = {
    { DXLControlTable::TemperatureLimit    , "TemperatureLimit" },
    { DXLControlTable::MaxVoltage          , "MaxVoltage" },
    { DXLControlTable::MinVoltage          , "MinVoltage" },
    { DXLControlTable::PWMLimit            , "PWMLimit" },
    { DXLControlTable::MaxCurrent          , "MaxCurrent" },
    { DXLControlTable::MaxVelocity         , "MaxVelocity" },
    { DXLControlTable::MaxPosition         , "MaxPosition" },
    { DXLControlTable::MinPosition         , "MinPosition" },
    { DXLControlTable::TorqueEnable        , "TorqueEnable" },
    { DXLControlTable::LED                 , "LED" },
    { DXLControlTable::StatusReturnLevel   , "StatusReturnLevel" },
    { DXLControlTable::VelocityIGain       , "VelocityIGain" },
    { DXLControlTable::VelocityPGain       , "VelocityPGain" },
    { DXLControlTable::PositionDGain       , "PositionDGain" },
    { DXLControlTable::PositionIGain       , "PositionIGain" },
    { DXLControlTable::PositionPGain       , "PositionPGain" },
    { DXLControlTable::Feedforward2Gain    , "Feedforward2Gain" },
    { DXLControlTable::Feedforward1Gain    , "Feedforward1Gain" },
    { DXLControlTable::BusWatchdog         , "BusWatchdog" },
    { DXLControlTable::GoalPWM             , "GoalPWM" },
    { DXLControlTable::GoalCurrent         , "GoalCurrent" },
    { DXLControlTable::GoalVelocity        , "GoalVelocity" },
    { DXLControlTable::ProfileAcceleration , "ProfileAcceleration" },
    { DXLControlTable::ProfileVelocity     , "ProfileVelocity" },
    { DXLControlTable::GoalPosition        , "GoalPosition" },
    { DXLControlTable::RealtimeTick        , "RealtimeTick" },
    { DXLControlTable::Moving              , "Moving" },
    { DXLControlTable::MovingStatus        , "MovingStatus" },
    { DXLControlTable::PresentPWM          , "PresentPWM" },
    { DXLControlTable::PresentCurrent      , "PresentCurrent" },
    { DXLControlTable::PresentVelocity     , "PresentVelocity" },
    { DXLControlTable::PresentPosition     , "PresentPosition" },
    { DXLControlTable::VelocityTrajectory  , "VelocityTrajectory" },
    { DXLControlTable::PresentTrajectory   , "PresentTrajectory" },
    { DXLControlTable::PresentInputVoltage , "PresentInputVoltage" },
    { DXLControlTable::PresentTemperature  , "PresentTemperature" }
};

const int IndirectAddressForReading = 168;
const int IndirectDataForReading = 224;

const int IndirectAddressForWriting = 578;
const int IndirectDataForWriting = 634;

/**
 * Each dynamixel motor in the group has an id.
 */
typedef int DXLId;

/**
 * We sometimes set and update lists of control table entries
 * for each dynamixel in the group. Each can be configured individually.
 * 
 * Here is an example:
 * 
 * {
 *  "5": [128, 132],    // PresentVelocity, PresentPosition
 *  "6": [128, 132],
 * }
 */
typedef map<DXLId, vector<DXLControlTable>> DXLIdsToControlTableEntries;

/**
 * A dynamixel motor in the group is configured to
 * read and write values for a subset of the control table. 
 * 
 * Here is an example:
 * 
 *  {
 *     "128": 3305,     // PresentVelocity
 *     "132": 2048,     // PresentPosition
 *  },
 */
typedef map<DXLControlTable, int> DeviceValues;

/**
 * Each dynamixel motor in the group has such a map.
 * 
 * Here is an example:
 * {
 *   "5": {
 *     "128": 3305,     // PresentVelocity
 *     "132": 2048,     // PresentPosition
 *   },
 *   "6": {
 *     "128": 1053,
 *     "132": 2056,
 *   }
 * }
 */
typedef map<DXLId, DeviceValues> DXLIdsToValues;

inline std::ostream& operator << (std::ostream& os, const DXLIdsToValues& values)
{
    os << "{";
    for (auto id_values : values) {
        os << id_values.first << ": {";
        for (auto entry_value : id_values.second) {
            DXLControlTable control_table_key = entry_value.first;
            os << to_underlying(control_table_key) << "(" << ControlTableEntriesToNames.at(control_table_key) << "): " << entry_value.second << ", ";
        }
        os << "}, ";
    }
    os << "}";
    return os;
}

/**
 * When we read and write to device, we often get
 * current times before and after the operation. 
 */
struct TimestampPair {
    double t0;
    double t1;
};

/**
 * The values that we read from each dynamixel in the
 * group for the configured control table entires,
 * and read min and max timestamps.
 */
struct DynamixelGroupState {
    DXLIdsToValues values;
    TimestampPair timestamp;

    void print_on(std::ostream& os) const;
    string to_string() const;
};


/**
 * A command should be populated by the user with 
 * values to write for each control table entry,
 * for each dynamixel. User populates and 
 * DynamixelGroupController takes care of writing 
 * to the device. Yes, it looks exactly the same
 * as DynamixelGroupState, but it will grow.
 */
struct DynamixelGroupCommand {

    // Want to write these values to dynamixel devices.
    DXLIdsToValues values;
    
    // Will NOT be used to write. Instead, will contain
    // the last known write timestamps, if they exist.
    TimestampPair timestamp = {0,0};

    // Can be used to control whether the command will 
    // be written to device at all.
    bool should_write = true;

    void print_on(std::ostream& os) const;
    string to_string() const;
};


/**
 * A DynamixelGroupController is an abstraction over a
 * group of dynamixel motors chained together. It provides
 * a low-level but convenient interface to run a synchronous
 * read-write loop to read from the device, update whatever
 * state, compute a resulting command, and then write that command
 * to the device.
 */
class DynamixelGroupController {
public:

    using Ptr = std::shared_ptr<DynamixelGroupController>;

    DynamixelGroupController(
        const string& device_name,
        int baud_rate);

    DynamixelGroupController(
        const string& device_name,
        int baud_rate,
        const vector<DXLId>& dxl_ids,
        const vector<OperatingMode>& operating_modes,
        const DXLIdsToControlTableEntries& read_control_map,
        const DXLIdsToControlTableEntries& write_control_map);

    DynamixelGroupController(
        const string& device_name,
        int baud_rate,
        const vector<DXLId>& dxl_ids,
        const OperatingMode operating_mode,
        const vector<DXLControlTable>& read_control_list,
        const vector<DXLControlTable>& write_control_list);

    static Ptr PositionController(
        const string& device_name,
        int baud_rate,
        const vector<DXLId>& dxl_ids) {
            auto k = new DynamixelGroupController(
                device_name,
                baud_rate,
                dxl_ids,
                OperatingMode::PositionControl,
                {DXLControlTable::PresentPosition},
                {DXLControlTable::GoalPosition}
            );
            return Ptr(k);
        }

    static Ptr VelocityController(
        const string& device_name,
        int baud_rate,
        const vector<DXLId>& dxl_ids) {
            auto k = new DynamixelGroupController(
                device_name,
                baud_rate,
                dxl_ids,
                OperatingMode::VelocityControl,
                {DXLControlTable::PresentVelocity, DXLControlTable::PresentPosition},
                {DXLControlTable::GoalVelocity}
            );
            return Ptr(k);
        }

    virtual ~DynamixelGroupController();

    // User passes a function of this type to 'run_readwrite_loop'.
    using ReadWriteLoopFunction = std::function<
        bool(                           // should return whether to continue or not
        const DynamixelGroupState&,     // current state of the group of dynamixels
        DynamixelGroupCommand&          // command to populate
        )>;

    // This function will read from the dynamixel group device,
    // call the give RealTimeFunction, and write the populated
    // command's values. It will issue blocking (sync) reading 
    // and writing calls to the device. It will do this over and
    // over in a loop, until the given function returns false.
    void run_readwrite_loop(ReadWriteLoopFunction f);


    // Enable and disable torque. Must be done bracketing changes
    // to operating modes.
    void enable_torque(const vector<DXLId> & dxl_ids);
    void disable_torque(const vector<DXLId> & dxl_ids);
    bool get_torque_enabled(DXLId dxl_id);

    // Set and get operating modes.
    void set_current_control(const vector<DXLId> & dxl_ids);
    void set_velocity_control(const vector<DXLId> & dxl_ids);
    void set_position_control(const vector<DXLId> & dxl_ids);
    void set_extended_position_control(const vector<DXLId> & dxl_ids);
    void set_position_current_control(const vector<DXLId> & dxl_ids);
    void set_pwm_control(const vector<DXLId> & dxl_ids);
    int get_operating_mode(DXLId dxl_id);

    // Configure how which control table entries will read and write.
    void set_sync_read(const DXLIdsToControlTableEntries &dxl_ids_to_values);
    void set_sync_write(const DXLIdsToControlTableEntries &dxl_ids_to_values);

    DXLIdsToControlTableEntries get_read_control_map() const { return sync_read_settings; }
    DXLIdsToControlTableEntries get_write_control_map() const { return sync_write_settings; }

    // You can read and write directly if you want.
    DynamixelGroupState read();
    void write(DynamixelGroupCommand &values_to_write);

    // We can also ping and reboot.
    vector<uint8_t> ping();
    bool reboot(DXLId dxl_id);

    // For direct-low level synchronous write operation, if you really want to.
    void write_directly(DXLId dxl_id, DXLControlTable control_table_entry, int value, bool disable_torque_=true);

    const string & get_device_name() const { return device_name; }
    int get_baud_rate() const { return baud_rate; }

    void freeze();

protected:

    void check_write_txrx(const string & name, DXLId dxl_id, int value, int control_table_index, int result, uint8_t dxl_error);
    void check_read_txrx(const string & name, DXLId dxl_id, int control_table_index, int result, uint8_t dxl_error);

    void write_1byte_txrx(DXLId dxl_id, int control_table_index, int value);
    void write_2byte_txrx(DXLId dxl_id, int control_table_index, int value);
    void write_4byte_txrx(DXLId dxl_id, int control_table_index, int value);

    int read_1byte_txrx(DXLId dxl_id, int control_table_index);
    int read_2byte_txrx(DXLId dxl_id, int control_table_index);
    int read_4byte_txrx(DXLId dxl_id, int control_table_index);

    void set_operating_mode(DXLId dxl_id, OperatingMode operating_mode);
    void set_operating_modes(const vector<DXLId> & dxl_ids, OperatingMode operating_mode);
    void set_operating_modes(const vector<DXLId> & dxl_ids, const vector<OperatingMode> & operating_modes);

    string device_name;
    int baud_rate;

    dynamixel::PortHandler* port_handler;
    dynamixel::PacketHandler* packet_handler;

    dynamixel::GroupSyncRead* sync_reader;
    dynamixel::GroupSyncWrite* sync_writer;

    DXLIdsToControlTableEntries sync_read_settings;
    DXLIdsToControlTableEntries sync_write_settings;
};


struct DynamixelException: public std::exception {
    string reason;
    DynamixelException(const string & reason): std::exception(), reason(reason) {}
    const char* what() const noexcept { return reason.c_str(); }
};


} // namespace dynamixelgroup
} // namespace roboflex

#endif // ROBOFLEX_DYNAMIXEL_CONTROLLER__H
