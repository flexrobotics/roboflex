#include <thread>
#include <chrono>
#include <limits>
#include <iomanip>
#include "flatbuffers/flexbuffers.h"
#include "roboflex_core/core_messages/core_messages.h"
#include "roboflex_core/util/utils.h"
#include "roboflex_core/util/get_process_memory_usage.h"
#include "roboflex_core/serialization/flex_utils.h"
#include "roboflex_core/core_nodes/metrics.h"

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

namespace roboflex {
namespace nodes {


// -- MetricTracker --

MetricTracker::MetricTracker():
    count(0),
    total(0),
    mean_value(0),
    m2_value(0),
    max_value(std::numeric_limits<double>::min()),
    min_value(std::numeric_limits<double>::max())
{

}

MetricTracker::MetricTracker(unsigned int count, double total, double mean_value, double m2_value, double max_value, double min_value):
    count(count),
    total(total),
    mean_value(mean_value),
    m2_value(m2_value),
    max_value(max_value),
    min_value(min_value)
{

}

void MetricTracker::print_on(ostream& os) const {
    os << "<MetricTracker"
       << " count:" << count
       << " total:" << total
       << " mean:" << mean_value
       << " variance:" << m2_value
       << " min:" << min_value
       << " max:" << max_value
       << ">";
}

std::string MetricTracker::to_string() const
{
    std::stringstream sst;
    print_on(sst);
    return sst.str();
}

void MetricTracker::pretty_print_on(ostream& os) const
{
    os << "mean: " << std::fixed << std::setprecision(4) << mean_value
       << " [" << std::fixed << std::setprecision(4) << min_value
       << ", " << std::fixed << std::setprecision(4) << max_value
       << "]  var: " << std::fixed << std::setprecision(4) << variance_value()
       << "]  total: " << std::fixed << std::setprecision(4) << total;
}

std::string MetricTracker::to_pretty_string() const
{
    std::stringstream sst;
    pretty_print_on(sst);
    return sst.str();
}

void MetricTracker::pretty_print(const std::string& title, bool compact) const
{
    if (compact) {
        std::cout << title << " " << std::fixed << std::setprecision(6) 
        << " μ:" << mean_value 
        << " σ²:" << variance_value()
        << " N:" << std::setprecision(3) << count;
        if (count > 0) {
            std::cout << std::setprecision(6) << " [" << min_value << "," << max_value << "]";
        }
    } else {
        if (title.length() > 0) {
            std::cout << title << std::endl;
        }
        std::cout 
            << std::string("    sample count: ") << count << std::endl
            << std::fixed << std::setprecision(6)
            << "        mean: " << mean_value << std::endl
            << "    variance: " << variance_value() << std::endl;
        if (count > 0) {
            std::cout 
                << "         min: " << min_value << std::endl
                << "         max: " << max_value << std::endl
                << "   sum total: " << total << std::endl;
        }
    }
}

void MetricTracker::record_value(double value)
{
    count += 1;
    total += value;

    double delta = value - mean_value;
    mean_value += (delta / count);
    double delta2 = value - mean_value;
    m2_value += (delta * delta2);

    min_value = std::min(min_value, value);
    max_value = std::max(max_value, value);
}

void MetricTracker::reset()
{
    count = 0;
    total = 0;
    mean_value = 0;
    m2_value = 0;
    max_value = std::numeric_limits<double>::min();
    min_value = std::numeric_limits<double>::max();
}


// -- MetricsMessage --

MetricsMessage::MetricsMessage(Message& other):
    Message(other)
{
    // just deserialize the whole map now....
    auto root = root_map();
    //elapsed_time = root["elapsed_time"].AsDouble();
    //current_mem_usage = root["current_mem_usage"].AsUInt64();
    auto keys = root.Keys();
    for (size_t i = 0; i < keys.size(); i++) {
        std::string name = keys[i].AsString().str();
        if (name != "_meta") {
            if (root[name].IsMap()) {
                auto submap = root[name].AsMap();
                metrics[name] = MetricTracker(
                    submap["count"].AsInt32(),
                    submap["total"].AsDouble(),
                    submap["mean"].AsDouble(),
                    submap["variance"].AsDouble(),
                    submap["max"].AsDouble(),
                    submap["min"].AsDouble()
                );
            }
        }
    }
}

MetricsMessage::MetricsMessage(
    double elapsed_time,
    const std::map<std::string, MetricTracker>& metrics,
    const uuid& parent_node_guid,
    const string& parent_node_name,
    const uuid& child_node_guid,
    const string& child_node_name,
    const string& host_name):
        Message(core::CoreModuleName, MetricsMessageType),
        metrics(metrics)
{
    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {

        fbb.String("host_name", host_name);
        fbb.String("parent_node_name", parent_node_name);
        fbb.String("child_node_name", child_node_name);

        serialization::serialize_uuid(parent_node_guid, "parent_node_guid", fbb);
        serialization::serialize_uuid(child_node_guid, "child_node_guid", fbb);

        fbb.Double("elapsed_time", elapsed_time);
        fbb.UInt("current_mem_usage", util::getCurrentRSS());

        for (auto const& x: metrics) {
            auto name = x.first;
            auto tracker = x.second;
            fbb.Map(name.c_str(), [&](){
                fbb.Int("count", tracker.count);
                fbb.Double("total", tracker.total);
                fbb.Double("mean", tracker.mean_value);
                fbb.Double("variance", tracker.variance_value());
                fbb.Double("max", tracker.max_value);
                fbb.Double("min", tracker.min_value);
            });
        }
    });
}

void MetricsMessage::print_on(ostream& os) const
{
    os << "<MetricsMessage "
       << " parent: " << parent_node_guid() << " \"" << parent_node_name() << "\""
       << " child: " << child_node_guid() << " \"" << child_node_name() << "\""
       << " elapsed_time:" << elapsed_time()
       << " current_mem_usage:" << current_mem_usage();
    for (auto const& [name, tracker]: metrics) {
        os << " " << name << ": " << tracker.to_string();
    }
    core::Message::print_on(os);
    os << ">";
}

void MetricsMessage::pretty_print_on(ostream& os) const
{
    double dt = metrics.at("dt").mean_value;
    os << std::fixed << std::setprecision(4)
       << "MetricsMessage " << std::endl;
    if (dt == 0) {
        os << "           frequency  NAN" << std::endl;
    } else {
        os << "           frequency  " << (1.0 / metrics.at("dt").mean_value) << std::endl;
    }
    os << "               count  " << metrics.at("time").count << std::endl
       << "   elapsed_time, sec  " << elapsed_time() << std::endl
       << "        my time, sec  " << time() << std::endl
       << "       time fraction  " << (metrics.at("time").count * metrics.at("time").mean_value) / elapsed_time() << std::endl
       << "       bytes per sec  " << (metrics.at("bytes").total) / elapsed_time() << std::endl
       << "   current_mem_usage  " << current_mem_usage() << std::endl
       << "    parent_node_name  " << parent_node_name() << std::endl
       << "     child_node_name  " << child_node_name() << std::endl
       << "    parent_node_guid  " << parent_node_guid() << std::endl
       << "     child_node_guid  " << child_node_guid() << std::endl
       << "           host_name  " << host_name() << std::endl;
    for (auto const& [name, tracker]: metrics) {
        os << setw(20 - name.size()) << " " << name << "  " << tracker.to_pretty_string() << std::endl;
    }
}

std::string MetricsMessage::to_pretty_string() const
{
    std::stringstream sst;
    pretty_print_on(sst);
    return sst.str();
}

void MetricsMessage::pretty_print(const std::string& title, bool compact) const
{
    pretty_print_on(std::cerr);
    // std::cout << title << source_node_name() << ": Metrics t = " << std::fixed << timestamp() << std::endl;
    // std::cout << "  FROM " << parent_node_guid() << " \"" << parent_node_name() << "\"  TO " << child_node_guid() << " \"" << child_node_name() << "\"" << std::endl;
    // std::cout << "        sample count: " << metrics.at("time").count << std::endl;
    // std::cout << std::fixed << std::setprecision(6)
    //     << "     elapsed time, seconds: " << elapsed_time() << std::endl
    //     << "  current mem usage, bytes: " << current_mem_usage() << std::endl
    //     << "             frequency, hz: " << (1.0 / metrics.at("dt").mean_value) << std::endl
    //     << "             time fraction: " << (metrics.at("time").count * metrics.at("time").mean_value) / elapsed_time() << std::endl
    //     << "             bytes per sec: " << (metrics.at("bytes").total) / elapsed_time() << std::endl;

    // // metrics.at("time"). pretty_print  ("     receive time, seconds: ", compact);
    // // std::cout << std::endl;
    // // metrics.at("bytes").pretty_print  ("                     bytes: ", compact);
    // // std::cout << std::endl;
    // metrics.at("dt").pretty_print     ("               dt, seconds: ", compact);
    // std::cout << std::endl;
    // metrics.at("latency").pretty_print("          latency, seconds: ", compact);
    // std::cout << std::endl;
    // metrics.at("missed").pretty_print ("           missed messages: ", compact);
    // std::cout << std::endl;

    // std::cout << std::endl;
}


// -- MetricsPublisherNode --

MetricsPublisherNode::MetricsPublisherNode(const std::string& name):
    Node(name)
{
    this->reset();

    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);
    host_name = hostname;
}

void MetricsPublisherNode::child_node_set(const uuid& node_guid, const string& node_name)
{
    if (!child_node_name.empty()) {
        throw std::runtime_error("Attempted to connect metrics node to more than one child node.");
    } else {
        child_node_name = node_name;
        child_node_guid = node_guid;
    }
}

void MetricsPublisherNode::parent_node_set(const uuid& node_guid, const string& node_name)
{
    if (!parent_node_name.empty()) {
        throw std::runtime_error("Attempted to connect metrics node to more than one parent node.");
    } else {
        parent_node_name = node_name;
        parent_node_guid = node_guid;
    }
}

void MetricsPublisherNode::receive_from(core::MessagePtr, const core::Node&)
{
    publish_and_reset();
}

void MetricsPublisherNode::record_metrics(double receive_time, long unsigned int bytes, double time_since_last_receive, double latency, int num_missed_messages)
{
    std::unique_lock<std::mutex> lck(mtx);

    tracked_receive_time.record_value(receive_time);
    tracked_bytes.record_value(bytes);
    if (time_since_last_receive != -1) {
        tracked_dt.record_value(time_since_last_receive);
    }
    tracked_latency.record_value(latency);
    tracked_missed_messages.record_value((double)num_missed_messages);
}

void MetricsPublisherNode::reset()
{
    last_reset_time = core::get_current_time();
    tracked_receive_time.reset();
    tracked_bytes.reset();
    tracked_dt.reset();
    tracked_latency.reset();
    tracked_missed_messages.reset();
}

void MetricsPublisherNode::publish_and_reset()
{
    std::unique_lock<std::mutex> lck(mtx);

    double elapsed_time = core::get_current_time() - last_reset_time;

    std::map<std::string, MetricTracker> m = {
        { std::string("time"), tracked_receive_time },
        { std::string("bytes"), tracked_bytes },
        { std::string("dt"), tracked_dt },
        { std::string("latency"), tracked_latency },
        { std::string("missed"), tracked_missed_messages }
    };

    this->signal(std::make_shared<MetricsMessage>(elapsed_time, m,
        parent_node_guid, parent_node_name, child_node_guid, child_node_name, host_name));

    this->reset();
}

void MetricsPublisherNode::pretty_print(const std::string& title, bool compact) const
{
    std::cout << title << ": Metrics" << std::endl;
    std::cout << "  sample count: " << tracked_receive_time.count << std::endl;
    double elapsed_time = core::get_current_time() - last_reset_time;
    std::cout << std::fixed << std::setprecision(6)
        << "  elapsed time, seconds: " << elapsed_time << std::endl
        << "  current mem usage, bytes: " << util::getCurrentRSS() << std::endl
        << "  frequency, hz: " << (1.0 / tracked_dt.mean_value) << std::endl
        << "  time fraction: " << (tracked_receive_time.count * tracked_receive_time.mean_value) / elapsed_time << std::endl;
    tracked_receive_time.pretty_print("  receive time, seconds", compact);
    tracked_bytes.pretty_print("  bytes", compact);
    tracked_dt.pretty_print("  dt, seconds", compact);
    tracked_latency.pretty_print("  latency, seconds", compact);
    tracked_missed_messages.pretty_print("  missed messages", compact);
}


// -- MetricsNode --

MetricsNode::MetricsNode(const std::string& name, const float passive_frequency_hz):
    Node(name),
    publisher_node(std::make_shared<MetricsPublisherNode>(name.substr(0, 28)+"_Pub")),
    last_receive_time(-1),
    passive_frequency_hz(passive_frequency_hz),
    last_passive_publish_time(0)
{

}

MetricsNode::MetricsNode(Node& publisher_target_node, const string& name, const float passive_frequency_hz):
    Node(name),
    publisher_node(std::make_shared<MetricsPublisherNode>(name.substr(0, 28)+"_Pub")),
    last_receive_time(-1),
    passive_frequency_hz(passive_frequency_hz),
    last_passive_publish_time(0)
{
    publisher_node->connect(publisher_target_node);
}

MetricsNode::MetricsNode(shared_ptr<Node> publisher_target_node, const string& name, const float passive_frequency_hz):
    Node(name),
    publisher_node(std::make_shared<MetricsPublisherNode>(name.substr(0, 28)+"_Pub")),
    last_receive_time(-1),
    passive_frequency_hz(passive_frequency_hz),
    last_passive_publish_time(0)
{
    publisher_node->connect(publisher_target_node);
}

void MetricsNode::on_connect(const core::Node& node, bool node_is_child)
{
    if (node_is_child) {
        publisher_node->child_node_set(node.get_guid(), node.get_name());
    } else {
        publisher_node->parent_node_set(node.get_guid(), node.get_name());
    }
}

void MetricsNode::receive(core::MessagePtr m)
{
    // Measure the time
    double t0 = core::get_current_time();

    // Propagate the data...
    signal(m);

    // Measure the time again:
    double t1 = core::get_current_time();

    // Now we know how long the downstream node takes in its receive call.
    double receive_dt = t1 - t0;

    // Also track time intervals between invocations.
    bool first_time = last_receive_time == -1;
    double time_since_last_receive = last_receive_time == -1 ? 0 : (t0 - last_receive_time);
    last_receive_time = t0;

    // Also track the number of bytes going through...
    long unsigned int bytes = m->get_raw_size();

    // Compute the latency: the current time minus
    // the message's timestamp (when it was created, or broadcast).
    double latency = t0 - m->timestamp();

    // compute the number of messages we skipped from that source
    auto guid = m->source_node_guid().str();
    auto new_message_index = m->message_counter();
    int last_received_message_index = 0;
    auto lmip = node_uuids_to_last_received_message_indexes.find(guid);
    if (lmip != node_uuids_to_last_received_message_indexes.end()) {
        last_received_message_index = lmip->second;
    }
    node_uuids_to_last_received_message_indexes[guid] = new_message_index;
    int missed_messages = std::max<int>(0, new_message_index - last_received_message_index - 1);

    // record all the above
    publisher_node->record_metrics(receive_dt, bytes, first_time ? -1 : time_since_last_receive, latency, missed_messages);


    // 'passive publishing' is just publishing that happens upon the
    // signaller's thread, if we haven't published for some time
    if (passive_frequency_hz != 0) {
        if (last_passive_publish_time == 0) {
            last_passive_publish_time = t0;
        }
        double time_since_last_passive_publish = t0 - last_passive_publish_time;
        double passive_period_seconds = 1.0 / passive_frequency_hz;
        if (time_since_last_passive_publish >= passive_period_seconds) {
            last_passive_publish_time = t0;
            publisher_node->publish_and_reset();
        }
    }
}

std::string MetricsNode::to_string() const
{
    return "<MetricsNode \"" + get_name() + "\">";
}

void MetricsNode::pretty_print(bool compact) const
{
    publisher_node->pretty_print(get_name(), compact);
}

} // namespace nodes
} // namespace roboflex
