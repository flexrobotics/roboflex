#ifndef ROBOFLEX_GRAPH_ROOT_NODE__H
#define ROBOFLEX_GRAPH_ROOT_NODE__H

#include "roboflex_core/node.h"
#include "roboflex_core/core_nodes/frequency_generator.h"

namespace roboflex {
using namespace core;
namespace nodes {

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
 *  git clone https://github.com/flexrobotics/roboflex_imgui.git
 *  cmake it, run "./message_central"
 * 
 * A GraphRoot can be configured with a custom publisher, to which you can 
 * publish metrics. We suggest an MQTT publisher, which is available in the
 * roboflex_transport_mqtt package. If you do that, then metrics_central can
 * be configured to subscribe to the same topic, and you can view the metrics.
 */
class GraphRoot: public RunnableNode {
public:

    GraphRoot(const string& name);

    GraphRoot(
        NodePtr metrics_publisher = nullptr,
        const float metrics_publishing_frequency_hz = 1.0,
        const string& name = "GraphRoot");

    void start() override;
    void start_all(const RunnableNodePtr node_to_run = nullptr);
    void profile(const RunnableNodePtr node_to_run = nullptr);
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


} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_GRAPH_ROOT_NODE__H
