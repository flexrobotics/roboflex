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
 * A child class of GraphController. It is designed to be
 * placed at the root of a graph of nodes, and that provides
 * profiling functions over the graph.
 * 
 * It can start() all RunnableNodes in the graph, and stop() them.
 * 
 * If started with profile=true, it inserts MetricsNodes in the graph
 * by breaking each connection with an intermediate MetricsNode. It
 * provides a frequency to trigger metrics publishing on, and it does
 * so by publishing to an mqtt broker. The graph then reports statistics
 * about performance of nodes and connections such as frequency, latency,
 * cpu usage, bytes/sec, etc.
 * 
 * These statistics can be viewed by running the 'message_central' program,
 * potentially on another computer:
 * 
 *  bazel run -c opt //metrics_central:metrics_central
 * 
 * You must be running an mqtt broker somewhere that both metrics_central and
 * the metrics-instrumented, running graph can access.
 * 
 * Alternatively, a Profiler can be configured with a custom publisher. 
 */
class Profiler: public nodes::GraphController {
public:

    Profiler(
        const string& mqtt_broker_address = "127.0.0.1",
        const int mqtt_broker_port = 1883,
        const string& mqtt_metrics_topic = "roboflexmetrics",
        const float metrics_publishing_frequency_hz = 1.0,
        const string& name = "Profiler");

    Profiler(
        shared_ptr<Node> metrics_publisher,
        const float metrics_publishing_frequency_hz = 1.0,
        const string& name = "Profiler");

    void start(bool profile = false);
    void stop() override;

    bool is_metrics_instrumented() const { return metrics_instrumented; }

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
