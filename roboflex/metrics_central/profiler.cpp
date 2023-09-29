#include "profiler.h"
#include "roboflex/core/core_nodes/metrics.h"
#include "roboflex/transport/mqtt/mqtt_nodes.h"

namespace roboflex {
namespace profiling {

using roboflex::nodes::MetricsNode;
using roboflex::nodes::FrequencyGenerator;

Profiler::Profiler(
    shared_ptr<Node> metrics_publisher,
    const float metrics_publishing_frequency_hz,
    const string& name):
        RunnableNode(name),
        metrics_instrumented(false),
        metrics_publishing_frequency_hz(metrics_publishing_frequency_hz),
        metrics_publisher(metrics_publisher)
{

}

Profiler::Profiler(
    const string& mqtt_broker_address,
    const int mqtt_broker_port,
    const string& mqtt_metrics_topic,
    const float metrics_publishing_frequency_hz,
    const string& name):
        Profiler::Profiler(
            std::make_shared<transportmqtt::MQTTPublisher>(
                transportmqtt::MakeMQTTContext(), 
                mqtt_broker_address, 
                mqtt_broker_port, 
                mqtt_metrics_topic),
            metrics_publishing_frequency_hz, 
            name)
{

}

void Profiler::start(bool profile)
{
    if (profile) {
        instrument_metrics();
        this->metrics_trigger->start();
    }

    this->walk_nodes_backwards([](NodePtr node, int depth){
        auto rn = std::dynamic_pointer_cast<RunnableNode>(node);
        if (rn) {
            rn->start();
        }
    });
}

void Profiler::stop()
{
    this->walk_nodes_forwards([](NodePtr node, int depth){
        auto rn = std::dynamic_pointer_cast<RunnableNode>(node);
        if (rn) {
            rn->stop();
        }
    });

    if (this->is_metrics_instrumented()) {
        this->metrics_trigger->stop();
        deinstrument_metrics();
    }
}

void Profiler::insert_metrics_between(NodePtr n1, NodePtr n2)
{
    // Create a metrics node
    string metrics_node_name = n1->get_name() + " -> " + n2->get_name();
    if (metrics_node_name.size() > 32) {
        metrics_node_name = n1->get_name().substr(0, 14) + " -> " + n2->get_name().substr(0, 14);
    }
    auto metrics_node = std::make_shared<MetricsNode>(metrics_node_name);

    // Disconnect the child from the node.
    n1->disconnect(n2);

    // Connect the node to the metrics node.
    n1->connect(metrics_node);

    // Connect the metrics node to the child.
    metrics_node->connect(n2);

    // Connect each metrics node to the metrics trigger and aggregator.
    this->metrics_trigger->connect(metrics_node->publisher_node);
    metrics_node->publisher_node->connect(this->metrics_aggregator);
}

void Profiler::instrument_metrics()
{
    if (this->is_metrics_instrumented()) {
        return;
    }

    // Create a frequency generator to trigger publishing
    this->metrics_trigger = std::make_shared<FrequencyGenerator>(
        metrics_publishing_frequency_hz, "MetricsPublishingTrigger");

    // Create an aggregator node to receive all results
    this->metrics_aggregator = std::make_shared<Node>("MetricsAggregator");

    //this->metrics_aggregator->connect(std::make_shared<nodes::MetricsPrinter>());
    
    // Maybe connect the aggregator to the publisher.
    if (this->metrics_publisher != nullptr) {
        this->metrics_aggregator->connect(this->metrics_publisher);
    }

    // Insert metrics nodes on each connection.
    this->walk_connections_backwards([this](NodePtr n1, NodePtr n2, int depth){
        this->insert_metrics_between(n1, n2);
    });

    this->metrics_instrumented = true;
}

void Profiler::deinstrument_metrics()
{
    if (!this->is_metrics_instrumented()) {
        return;
    }

    // Remove all Metrics Nodes
    this->filter_nodes([this](NodePtr n, int depth){
        return this->test_and_remove_metrics_node(n);
    });

    // Remove all else
    this->metrics_aggregator = nullptr;
    this->metrics_trigger = nullptr;
    this->metrics_instrumented = false;
}

bool Profiler::test_and_remove_metrics_node(NodePtr node)
{
    auto metrics_node = std::dynamic_pointer_cast<MetricsNode>(node);
    if (metrics_node == nullptr) {
        // not a MetricsNode - returning true makes the filter algorithm
        // keep this node. 
        return true;
    } else {
        // remove from the metrics trigger and aggregator
        this->metrics_trigger->disconnect(metrics_node);
        metrics_node->publisher_node->disconnect(this->metrics_aggregator);
        // return false: this node will be sectioned from the graph and deleted.
        return false;
    }
}

} // namespace profiling
} // namespace roboflex
