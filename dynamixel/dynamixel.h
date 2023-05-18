#ifndef ROBOFLEX_DYNAMIXEL__H
#define ROBOFLEX_DYNAMIXEL__H

#include <atomic>
#include <mutex>
#include "core/core.h"
#include "dynamixel_controller.h"

using std::atomic, std::string, std::shared_ptr;

namespace roboflex {
using namespace dynamixelgroup;
namespace dynamixelnodes {

constexpr char ModuleName[] = "dynamixel";

// --- Messages ---

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
        const string& device_name,
        int baud_rate,
        const string& name = "DynamixelGroup");

    DynamixelGroupController controller;

    void receive(core::MessagePtr m) override;

    bool readwrite_loop_function(const DynamixelGroupState& state, DynamixelGroupCommand& command);

protected:

    void child_thread_fn() override;

    // This isn't supported until gcc 12.1...
    //atomic<shared_ptr<DynamixelGroupCommandMessage>> last_command_message;

    std::recursive_mutex last_command_message_mutex;
    shared_ptr<DynamixelGroupCommandMessage> last_command_message = nullptr;  
};

} // dynamixelnodes
} // roboflex

#endif // ROBOFLEX_DYNAMIXEL__H
