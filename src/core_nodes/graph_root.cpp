#include "roboflex_core/core_nodes/graph_root.h"
#include "roboflex_core/core_nodes/metrics.h"

namespace roboflex {
namespace nodes {

GraphRoot::GraphRoot(const string& name):
    RunnableNode(name),
    metrics_instrumented(false)
{

}

GraphRoot::GraphRoot(
    shared_ptr<Node> metrics_publisher,
    const float metrics_publishing_frequency_hz,
    const string& name):
        RunnableNode(name),
        metrics_instrumented(false),
        metrics_publishing_frequency_hz(metrics_publishing_frequency_hz),
        metrics_publisher(metrics_publisher)
{

}

void GraphRoot::start(bool profile)
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

void GraphRoot::stop()
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

void GraphRoot::insert_metrics_between(NodePtr n1, NodePtr n2)
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

void GraphRoot::instrument_metrics()
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

void GraphRoot::deinstrument_metrics()
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

bool GraphRoot::test_and_remove_metrics_node(NodePtr node)
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
