#include "roboflex_core/core_nodes/graph_root.h"
#include "roboflex_core/core_nodes/metrics.h"

namespace roboflex {
namespace nodes {

GraphRoot::GraphRoot(
    const float metrics_printing_frequency_hz,
    const string& name, 
    bool debug):
        GraphRoot(
            metrics_printing_frequency_hz > 0 ? std::make_shared<nodes::MetricsPrinter>() : nullptr, 
            metrics_printing_frequency_hz, name, debug)
{

}

GraphRoot::GraphRoot(
    shared_ptr<Node> metrics_publisher,
    const float metrics_publishing_frequency_hz,
    const string& name,
    bool debug):
        RunnableNode(name),
        metrics_instrumented(false),
        metrics_publishing_frequency_hz(metrics_publishing_frequency_hz),
        metrics_publisher(metrics_publisher),
        debug(debug)
{

}

void GraphRoot::start() 
{
    start_all(nullptr);
}

void GraphRoot::start_all(RunnableNodePtr node_to_run) 
{
    this->_node_to_run = node_to_run;

    this->walk_nodes_backwards([node_to_run, debug=debug](NodePtr node, int){
        auto rn = std::dynamic_pointer_cast<RunnableNode>(node);
        if (rn && rn != node_to_run) {
            if (debug) {
                std::cerr << "GraphRoot starting " << rn->get_name() << "\n";
            }
            rn->start();
        }
    });

    if (node_to_run) {
        if (debug) {
            std::cerr << "GraphRoot running " << node_to_run->get_name() << "\n";
        }
        node_to_run->run();
    }
}

void GraphRoot::profile(RunnableNodePtr node_to_run) 
{
    instrument_metrics();
    if (this->metrics_trigger != nullptr) {
        this->metrics_trigger->start();
    }
    start_all(node_to_run);

    // If we're running a node as the main thread, 
    // then when we get here we're done, so stop
    // the graph.
    if (node_to_run != nullptr) {
        stop();
    }
}

void GraphRoot::stop()
{
    this->walk_nodes_forwards([debug=debug, node_to_run=this->_node_to_run](NodePtr node, int){
        auto rn = std::dynamic_pointer_cast<RunnableNode>(node);
        if (rn && rn != node_to_run) {
            if (debug) {
                std::cerr << "GraphRoot stopping " << rn->get_name() << "\n";
            }
            rn->stop();
        }
    });

    if (this->is_metrics_instrumented()) {
        if (this->metrics_trigger != nullptr) {
            this->metrics_trigger->stop();
        }
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
    if (this->metrics_trigger != nullptr) {
        this->metrics_trigger->connect(metrics_node->publisher_node);
    }
    metrics_node->publisher_node->connect(this->metrics_aggregator);
}

void GraphRoot::instrument_metrics()
{
    if (this->is_metrics_instrumented()) {
        return;
    }

    // Create a frequency generator to trigger publishing
    if (metrics_publishing_frequency_hz > 0) {
        this->metrics_trigger = std::make_shared<FrequencyGenerator>(
            metrics_publishing_frequency_hz, "MetricsPublishingTrigger");
    }

    // Create an aggregator node to receive all results
    this->metrics_aggregator = std::make_shared<Node>("MetricsAggregator");

    //this->metrics_aggregator->connect(std::make_shared<nodes::MetricsPrinter>());
    
    // Maybe connect the aggregator to the publisher.
    if (this->metrics_publisher != nullptr) {
        this->metrics_aggregator->connect(this->metrics_publisher);
    }

    // Insert metrics nodes on each connection.
    this->walk_connections_backwards([this](NodePtr n1, NodePtr n2, int){
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
    this->filter_nodes([this](NodePtr n, int){
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
        if (this->metrics_trigger != nullptr) {
            this->metrics_trigger->disconnect(metrics_node);
        }
        metrics_node->publisher_node->disconnect(this->metrics_aggregator);
        // return false: this node will be sectioned from the graph and deleted.
        return false;
    }
}

} // namespace profiling
} // namespace roboflex
