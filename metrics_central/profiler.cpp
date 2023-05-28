#include "profiler.h"
#include "core/core_nodes/metrics.h"
#include "transport/mqtt/mqtt_nodes.h"

namespace roboflex {
namespace profiling {

using roboflex::nodes::MetricsNode;
using roboflex::nodes::FrequencyGenerator;

Profiler::Profiler(
    shared_ptr<Node> metrics_publisher,
    const float metrics_publishing_frequency_hz,
    const string& name):
        nodes::GraphController(name),
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
        Profiler(
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

    nodes::GraphController::start();
}

void Profiler::stop()
{
    nodes::GraphController::stop();

    if (this->get_metrics_instrumented()) {
        this->metrics_trigger->stop();
        deinstrument_metrics();
    }
}

void Profiler::instrument_metrics()
{
    if (this->get_metrics_instrumented()) {
        return;
    }

    this->metrics_instrumented = true;

    this->metrics_trigger = std::make_shared<FrequencyGenerator>(
        metrics_publishing_frequency_hz, "MetricsPublishingTrigger");
    this->metrics_aggregator = std::make_shared<Node>("MetricsAggregator");

    //this->metrics_aggregator->connect(std::make_shared<nodes::MetricsPrinter>());
    
    if (this->metrics_publisher != nullptr) {
        this->metrics_aggregator->connect(this->metrics_publisher);
    }

    this->walk_connections_backwards([this](NodePtr n1, NodePtr n2){
        this->insert_metrics_between(n1, n2);
    });
}

void Profiler::insert_metrics_between(NodePtr n1, NodePtr n2)
{
    // Create a metrics node
    string metrics_node_name = n1->get_name() + " -> " + n2->get_name();
    if (metrics_node_name.size() > 32) {
        metrics_node_name = n1->get_name().substr(0, 14) + " -> " + n2->get_name().substr(0, 14);
    }
    shared_ptr<MetricsNode> metrics_node = std::make_shared<MetricsNode>(metrics_node_name);

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

void Profiler::deinstrument_metrics()
{
    if (!this->get_metrics_instrumented()) {
        return;
    }

    this->filter_nodes([this](NodePtr n){
        return this->test_and_remove_metrics_node(n);
    });

    this->metrics_instrumented = false;
    this->metrics_aggregator = nullptr;
    this->metrics_trigger = nullptr;
}

bool Profiler::test_and_remove_metrics_node(NodePtr node)
{
    shared_ptr<MetricsNode> metrics_node = std::dynamic_pointer_cast<MetricsNode>(node);
    if (metrics_node == nullptr) {
        return true;
    } else {
        this->metrics_trigger->disconnect(metrics_node);
        metrics_node->publisher_node->disconnect(this->metrics_aggregator);
        return false;
    }
}

} // namespace profiling
} // namespace roboflex
