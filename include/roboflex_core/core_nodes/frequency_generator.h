#ifndef ROBOFLEX_FREQUENCY_GENERATOR_NODE__H
#define ROBOFLEX_FREQUENCY_GENERATOR_NODE__H

#include <iostream>
#include <atomic>
#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * This node signals BlankMessage at the frequency specified 
 * in the constructor.
 * 
 * Supports sub-classing, since child nodes might want
 * to just "be" a frequency trigger and override on_trigger. 
 */
class FrequencyGenerator: public RunnableNode {
public:
    FrequencyGenerator(
        const float frequency_hz,
        const std::string& name = "FrequencyGenerator");

    void set_frequency(float new_frequency_hz);
    float get_frequency() const { return frequency_hz;  }

    MessagePtr handle_rpc(MessagePtr rpc_message) override;
    std::string to_string() const override;

protected:
    void child_thread_fn() override;

    virtual void on_trigger(double wall_clock_time);

    std::atomic<float> frequency_hz;
    uint32_t invocation_count;
};

// some rpc names
constexpr char GetFrequency[] = "get_frequency";
constexpr char GotFrequency[] = "got_frequency";
constexpr char SetFrequency[] = "set_frequency";

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_FREQUENCY_GENERATOR_NODE__H
