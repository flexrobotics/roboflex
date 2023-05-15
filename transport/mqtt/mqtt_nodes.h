#ifndef ROBOFLEX_TRANSPORT_MQTT_NODES__H
#define ROBOFLEX_TRANSPORT_MQTT_NODES__H

#include <memory>
#include <mosquitto.h>
#include "core/core.h"

namespace roboflex {
namespace transportmqtt {

using std::string, std::shared_ptr, std::cout;

class MQTTContext {
public:
    MQTTContext() {
        mosquitto_lib_init();
    }
    ~MQTTContext() {
        mosquitto_lib_cleanup();
    }
};

using MQTTContextPtr = shared_ptr<MQTTContext>;

inline MQTTContextPtr MakeMQTTContext() {
    int major, minor, revision;
    mosquitto_lib_version(&major, &minor, &revision);
    cout << "Made a new MQTT Context with mosquitto version "
         << major << "."
         << minor << "."
         << revision << endl;
    return make_shared<MQTTContext>();
}

class MQTTNodeBase {
public:
    MQTTNodeBase(
        MQTTContextPtr context,
        const string& broker_address,
        int broker_port,
        const string& topic_name,
        int keepalive_seconds,
        int qos,
        bool debug);

    virtual ~MQTTNodeBase();

    virtual void on_connect(struct mosquitto* m, int reason_code);
    virtual void on_disconnect(struct mosquitto* m, int reason_code);

    bool is_connected() const { return connected; }

    string get_broker_address() const { return broker_address; }
    int get_broker_port() const { return broker_port; }
    int get_keepalive_seconds() const { return keepalive_seconds; }
    string get_topic_name() const { return topic_name; }
    void set_topic_name(const std::string& t) { topic_name = t; }
    int get_qos() const { return qos; }
    bool get_debug() const { return debug; }

protected:
    MQTTContextPtr context;
    string broker_address;
    int broker_port;
    string topic_name;
    int keepalive_seconds;
    struct mosquitto* mosq;
    std::atomic<bool> connected;
    int qos;
    bool debug;
};

class MQTTPublisher: public core::Node, public MQTTNodeBase {
public:
    MQTTPublisher(
        MQTTContextPtr mqtt_context,
        const string& broker_address,
        int broker_port,
        const string& topic_name,
        const string& name = "MQTTPublisher",
        int keepalive_seconds = 60,
        int qos = 0,
        bool retained = false,
        bool debug = false);

    void receive(core::MessagePtr m) override;
    void publish(core::MessagePtr m) { this->signal_self(m); }
    bool get_retained() const { return retained; }

    void on_connect(struct mosquitto* m, int reason_code) override;
    void on_publish(struct mosquitto* m, int message_id);

protected:
    bool retained;
};

using MQTTPublisherPtr = shared_ptr<MQTTPublisher>;


class MQTTSubscriber: public core::RunnableNode, public MQTTNodeBase {
public:
    MQTTSubscriber(
        MQTTContextPtr mqtt_context,
        const string& broker_address,
        int broker_port,
        const string& topic_name,
        const string& name = "MQTTSubscriber",
        int keepalive_seconds = 60,
        int qos = 0,
        int loop_timeout_milliseconds = 100,
        bool debug = false);

    int get_loop_timeout_milliseconds() const { return loop_timeout_milliseconds; }
    bool is_subscribed() const { return subscribed; }

    // mqtt callbacks
    void on_connect(struct mosquitto* m, int reason_code) override;
    void on_subscribe(struct mosquitto *mosq, int mid, int qos_count, const int *granted_qos);
    void on_receive(struct mosquitto *mosq, const struct mosquitto_message *msg);


protected:
    void child_thread_fn() override;

    int loop_timeout_milliseconds;
    std::atomic<bool> subscribed;
};

using MQTTSubscriberPtr = shared_ptr<MQTTSubscriber>;

} // namespace transportmqtt
} // namespace roboflex

#endif // ROBOFLEX_TRANSPORT_MQTT_NODES__H

