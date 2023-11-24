#ifndef ROBOFLEX_METRICS_NODE__H
#define ROBOFLEX_METRICS_NODE__H

#include <iostream>
#include <mutex>
#include <map>
#include "roboflex_core/node.h"
#include "roboflex_core/serialization/flex_utils.h"
#include "roboflex_core/util/uuid.h"

namespace roboflex {
namespace nodes {

using std::map, std::string, sole::uuid;


/**
 * Client calls record_value multiple times, passing in a double value,
 * and this class tracks the mean, min, max, count, sum, and variance of that value.
 */
struct MetricTracker {
    MetricTracker();
    MetricTracker(unsigned int count, double total, double mean_value, double m2_value, double max_value, double min_value);
    void record_value(double value);
    void reset();
    double variance_value() const { return count == 0 ? count : m2_value/count; };

    void print_on(ostream& os) const;
    void pretty_print_on(ostream& os) const;
    string to_string() const;
    string to_pretty_string() const;
    void pretty_print(const std::string& title="", bool compact=true) const;

    unsigned int count;
    double total;
    double mean_value;
    double m2_value;
    double max_value;
    double min_value;
};


/**
 * A message that contains a dictionary of tracked metrics
 */
struct MetricsMessage: public core::Message {

    constexpr static char MetricsMessageType[] = "MetricsMessage";

    MetricsMessage(Message& other);
    MetricsMessage(
        double elapsed_time,
        const map<string, MetricTracker>& metrics,
        const uuid& parent_node_guid,
        const string& parent_node_name,
        const uuid& child_node_guid,
        const string& child_node_name,
        const string& host_name);

    void print_on(ostream& os) const override;
    void pretty_print_on(ostream& os) const;
    string to_pretty_string() const;
    void pretty_print(const string& title, bool compact) const;

    map<string, MetricTracker> metrics;

    double elapsed_time() const {
        return root_map()["elapsed_time"].AsDouble();
    }

    double time() const {
        return root_map()["time"].AsDouble();
    }

    uint64_t current_mem_usage() const {
        return root_map()["current_mem_usage"].AsUInt64();
    }

    const string parent_node_name() const {
        return root_map()["parent_node_name"].AsString().str();
    }

    const string child_node_name() const {
        return root_map()["child_node_name"].AsString().str();
    }

    const uuid parent_node_guid() const {
        auto blob = root_map()["parent_node_guid"].AsBlob();
        return serialization::deserialize_uuid(blob);
    }

    const uuid child_node_guid() const {
        auto blob = root_map()["child_node_guid"].AsBlob();
        return serialization::deserialize_uuid(blob);
    }

    const string host_name() const {
        return root_map()["host_name"].AsString().str();
    }
};


/**
 * A node that publishes metrics about the data it receives.
 * This Node is owned by a MetricsNode, and performs the actual data
 * aggregation when that MetricsNode calls record_metrics.
 * Somebody somewhere must either:
 *   1. call "publish_and_reset", or
 *   2. simply send this node any message at all,
 * in order to actually signal the metrics data.
 */
class MetricsPublisherNode: public core::Node {
public:
    MetricsPublisherNode(const string &name = "MetricsPublisherNode");

    void receive_from(core::MessagePtr m, const core::Node& from) override;

    // Called by MetricsNode
    void record_metrics(double receive_time, long unsigned int bytes, double time_since_last_receive, double latency, int num_missed_messages);

    // Called by whoever (and maybe by MetricsNode)
    void publish_and_reset();
    void reset();
    void pretty_print(const string& title="", bool compact=false) const;

    void child_node_set(const uuid& node_guid, const string& node_name);
    void parent_node_set(const uuid& node_guid, const string& node_name);

protected:

    MetricTracker tracked_receive_time;
    MetricTracker tracked_bytes;
    MetricTracker tracked_dt;
    MetricTracker tracked_latency;
    MetricTracker tracked_missed_messages;

    double last_reset_time;

    // Because whoever calls publish_and_reset is probably
    // operating on a different thread than whoever is ultimately
    // propagating data to here, and calling record_metrics.
    std::mutex mtx;

    uuid parent_node_guid;
    uuid child_node_guid;
    std::string parent_node_name;
    std::string child_node_name;
    std::string host_name;
};


/**
 * A node that:
 *  1. just signals whatever it receives, and
 *  2. collects and publishes metrics about what it receives data
 *      via it's owned publisher_node.
 * This is the Node that can be injected between other nodes,
 * and will record and can publish information about that link.
 */
class MetricsNode: public core::Node {
public:
    MetricsNode(
        const string& name = "MetricsNode",
        const float passive_frequency_hz = 0);

    MetricsNode(
        Node& publisher_target_node,
        const string& name = "MetricsNode",
        const float passive_frequency_hz = 0);

    MetricsNode(
        shared_ptr<Node> publisher_target_node,
        const string& name = "MetricsNode",
        const float passive_frequency_hz = 0);

    void receive(core::MessagePtr m) override;

    void pretty_print(bool compact=false) const;
    void reset() { publisher_node->reset(); }
    string to_string() const override;

    shared_ptr<MetricsPublisherNode> publisher_node;
    double last_receive_time;
    float passive_frequency_hz;
    double last_passive_publish_time;
    map<string, uint64_t> node_uuids_to_last_received_message_indexes;

protected:

    void on_connect(const core::Node& node, bool node_is_child) override;
};


/**
 * A node that pretty-prints metrics messages. Connect
 * it to a MetricsNode's publisher_node.
 */
class MetricsPrinter: public core::Node {
public:
    MetricsPrinter(bool compact=true, const string& name = "MetricsPrinter"):
        core::Node(name), compact(compact) {}

    void receive(core::MessagePtr m) override {
        if (m->message_name() == MetricsMessage::MetricsMessageType) {
            //MetricsMessage(*m).pretty_print(get_name(), compact);
            MetricsMessage(*m).pretty_print_on(std::cerr);
        } 
        this->signal(m);
    }

    bool compact;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_METRICS_NODE__H
