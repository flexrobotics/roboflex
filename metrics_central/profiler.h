#ifndef ROBOFLEX_PROFILER__H
#define ROBOFLEX_PROFILER__H

#include "core/node.h"
#include "core/core_nodes/frequency_generator.h"
#include "core/core_nodes/graph_control.h"

namespace roboflex {
using namespace core;
namespace profiling {

using std::string, std::set;

/**
 *
 */
class Profiler: public nodes::GraphController {
public:

    Profiler(
        shared_ptr<Node> metrics_publisher,
        const float metrics_publishing_frequency_hz = 1.0,
        const string& name = "Profiler");

    Profiler(
        const string& mqtt_broker_address = "127.0.0.1",
        const int mqtt_broker_port = 1883,
        const string& mqtt_metrics_topic = "roboflexmetrics",
        const float metrics_publishing_frequency_hz = 1.0,
        const string& name = "Profiler");

    void start(bool profile = false);
    void stop() override;

    bool get_metrics_instrumented() const { return metrics_instrumented; }

protected:

    void instrument_metrics();
    void deinstrument_metrics();

    void insert_metrics_between(NodePtr n1, NodePtr n2);
    bool test_and_remove_metrics_node(NodePtr node);

    bool metrics_instrumented = false;
    float metrics_publishing_frequency_hz;
    shared_ptr<nodes::FrequencyGenerator> metrics_trigger;
    shared_ptr<Node> metrics_aggregator;
    shared_ptr<Node> metrics_publisher;
};

} // namespace profiling
} // namespace roboflex

#endif // ROBOFLEX_PROFILER__H
