#ifndef ROBOFLEX_DYNAMIXEL__H
#define ROBOFLEX_DYNAMIXEL__H

#include <atomic>
#include <mutex>
#include "core/core.h"
#include "core/core_nodes/frequency_generator.h"
#include "dynamixel_controller.h"

using std::atomic, std::string, std::shared_ptr;

namespace roboflex {
using namespace dynamixelgroup;
namespace dynamixelnodes {

constexpr char ModuleName[] = "dynamixel";

// --- Messages ---

/**
 * @brief DynamixelGroupStateMessage is a message that contains a DynamixelGroupState.
 * Here is an example. It describes the state of two dynamixel motors, with ids 5 and 6.
 * For each motor, it contains the current velocity (128) and the current position (132).
 * 
 * "state": {
 *   "5": {
 *     "128": 3305,
 *     "132": 2048,
 *   },
 *   "6": {
 *     "128": 1053,
 *     "132": 2056,
 *   }
 * } 
 */
class DynamixelGroupStateMessage: public core::Message {
public:
    inline static const char MessageName[] = "DynamixelGroupStateMessage";
    DynamixelGroupStateMessage(core::Message& other): core::Message(other) {}
    DynamixelGroupStateMessage(const DynamixelGroupState& state);
    DynamixelGroupState get_state() const;
    void print_on(ostream& os) const override;
protected:
    mutable DynamixelGroupState _state;
    mutable bool _state_initialized = false;
};

/**
 * @brief DynamixelGroupStateMessage is a message that contains a DynamixelGroupCommand.
 * Here is an example. It describes the desired command for two dynamixel motors, with 
 * ids 5 and 6. It specifies the desired GoalPosition (116) for each motor.
 * 
 * "command": {
 *   "5": {
 *     "116": 3305,
 *   },
 *   "6": {
 *     "116": 1053,
 *   }
 * } 
 */
class DynamixelGroupCommandMessage: public core::Message {
public:
    inline static const char MessageName[] = "DynamixelGroupCommandMessage";
    DynamixelGroupCommandMessage(core::Message& other): core::Message(other) {}
    DynamixelGroupCommandMessage(const DynamixelGroupCommand& command);
    DynamixelGroupCommand get_command() const;
    void print_on(ostream& os) const override;
protected:
    mutable DynamixelGroupCommand _command;
    mutable bool _command_initialized = false;
};


// --- Nodes ---

class DynamixelGroupNode: public core::RunnableNode {
public:
    DynamixelGroupNode(
        DynamixelGroupController::Ptr controller,
        const string& name = "DynamixelGroup");

    DynamixelGroupController::Ptr controller;

    void receive(core::MessagePtr m) override;

    bool readwrite_loop_function(const DynamixelGroupState& state, DynamixelGroupCommand& command);

protected:

    void child_thread_fn() override;

    // This isn't supported until gcc 12.1...
    //atomic<shared_ptr<DynamixelGroupCommandMessage>> last_command_message;

    std::recursive_mutex last_command_message_mutex;
    shared_ptr<DynamixelGroupCommandMessage> last_command_message = nullptr;  
};

class DynamixelRemoteController: public core::Node {
public:
    DynamixelRemoteController(const string& name = "DynamixelRemoteController");
    void receive(core::MessagePtr m) override;
    virtual DXLIdsToValues readwrite_loop_function(const DynamixelGroupState& state) = 0;
};

class DynamixelRemoteFrequencyController: public nodes::FrequencyGenerator {
public:
    DynamixelRemoteFrequencyController(
        const float frequency_hz,
        const string& name = "DynamixelRemoteFrequencyController");
    void receive(core::MessagePtr m) override;
    void on_trigger(double wall_clock_time) override;
    virtual DXLIdsToValues readwrite_loop_function(const DynamixelGroupState& state) = 0;
protected:
    DynamixelGroupState state;
};

} // dynamixelnodes
} // roboflex

#endif // ROBOFLEX_DYNAMIXEL__H
