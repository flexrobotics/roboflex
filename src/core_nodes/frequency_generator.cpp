#include <thread>
#include <chrono>
#include "roboflex_core/core_messages/core_messages.h"
#include "roboflex_core/core_nodes/frequency_generator.h"

namespace roboflex {
namespace nodes {

FrequencyGenerator::FrequencyGenerator(
    const float frequency_hz,
    const std::string &name):
        RunnableNode(name),
        frequency_hz(frequency_hz),
        invocation_count(0)
{
    assert(frequency_hz != 0);
}

void FrequencyGenerator::set_frequency(float new_frequency_hz)
{
    frequency_hz = new_frequency_hz;
    assert(frequency_hz != 0);
}

MessagePtr FrequencyGenerator::handle_rpc(MessagePtr rpc_message)
{
    if (rpc_message->module_name() == CoreModuleName) {

        const auto message_name = rpc_message->message_name();

        if (message_name == GetFrequency) {
            return make_shared<FloatMessage>(GotFrequency, frequency_hz);
        }
        else if (message_name == SetFrequency) {
            this->set_frequency(FloatMessage(*rpc_message).value());
            return make_shared<BlankMessage>(OKMessageName);
        }
    }

    // let (force?) superclass to handle it
    return RunnableNode::handle_rpc(rpc_message);
}

void FrequencyGenerator::child_thread_fn()
{
    float prev_frequency = frequency_hz;

    auto start_t = std::chrono::steady_clock::now();

    while (!this->stop_requested()) {

        // The frequency_hz can change out from under us at any time.
        // So, if we find that the frequency_hz HAS changed, then
        // reset the start time and invocation count, so we operate correctly.
        float current_frequency = frequency_hz;
        if (current_frequency != prev_frequency) {
            start_t = std::chrono::steady_clock::now();
            invocation_count = 0;
            prev_frequency = current_frequency;
        }

        if (frequency_hz > 0) {
            sleep_until_next_interval(start_t, 1.0 / this->frequency_hz);
        }

        // ------------------------------
        // do whatever we actually want to do.

        this->on_trigger(get_current_time());

        invocation_count += 1;
    }
}

void FrequencyGenerator::on_trigger(double /*wall_clock_time*/)
{
    // I will simply signal down-stream, but child classes may override
    this->signal(std::make_shared<BlankMessage>("FrequencyTrigger"));
}

std::string FrequencyGenerator::to_string() const
{
    return "<FrequencyGenerator hz=" + std::to_string(frequency_hz) + " " + RunnableNode::to_string() + ">";
}

} // namespace nodes
} // namespace roboflex
