#include "transport/mqtt/mqtt_nodes.h"
#include "core/message_backing_store.h"
#include "core/util/utils.h"


namespace roboflex {
namespace transportmqtt {


void global_publisher_on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    static_cast<MQTTPublisher*>(obj)->on_connect(mosq, reason_code);
}

void global_subscriber_on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    static_cast<MQTTSubscriber*>(obj)->on_connect(mosq, reason_code);
}

void global_publisher_on_disconnect(struct mosquitto *mosq, void *obj, int reason_code)
{
    static_cast<MQTTPublisher*>(obj)->on_disconnect(mosq, reason_code);
}

void global_subscriber_on_disconnect(struct mosquitto *mosq, void *obj, int reason_code)
{
    static_cast<MQTTSubscriber*>(obj)->on_disconnect(mosq, reason_code);
}

void global_on_publish(struct mosquitto *mosq, void *obj, int message_id)
{
    static_cast<MQTTPublisher*>(obj)->on_publish(mosq, message_id);
}

void global_on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    static_cast<MQTTSubscriber*>(obj)->on_subscribe(mosq, mid, qos_count, granted_qos);
}

void global_on_receive(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    static_cast<MQTTSubscriber*>(obj)->on_receive(mosq, msg);
}


// -- MQTTNodeBase --

MQTTNodeBase::MQTTNodeBase(
    MQTTContextPtr context,
    const string& broker_address,
    int broker_port,
    const string& topic_name,
    int keepalive_seconds,
    int qos,
    bool debug):
        context(context),
        broker_address(broker_address),
        broker_port(broker_port),
        topic_name(topic_name),
        keepalive_seconds(keepalive_seconds),
        mosq(nullptr),
        connected(false),
        qos(qos),
        debug(debug)
{

}

MQTTNodeBase::~MQTTNodeBase()
{
    if (mosq != nullptr) {
        mosquitto_destroy(mosq);
    }
}

void MQTTNodeBase::on_disconnect(struct mosquitto* m, int reason_code)
{
    if (debug) {
        cout << "MQTTNodeBase on_disconnect: " << mosquitto_connack_string(reason_code) << std::endl;
    }

    connected = false;
}

void MQTTNodeBase::on_connect(struct mosquitto* m, int reason_code)
{
    // we're gonna need this callback...
}


// -- MQTTPublisher --

MQTTPublisher::MQTTPublisher(
    MQTTContextPtr mqtt_context,
    const string& broker_address,
    int broker_port,
    const string& topic_name,
    const string& name,
    int keepalive_seconds,
    int qos,
    bool retained,
    bool debug):
        core::Node(name),
        MQTTNodeBase(mqtt_context, broker_address, broker_port, topic_name, keepalive_seconds, qos, debug),
        retained(retained)
{
    // Construct an mqtt session.
    const char * client_id = NULL;
    bool clean_session = true;
    mosq = mosquitto_new(client_id, clean_session, this);
    if (mosq == nullptr) {
        throw std::runtime_error("MQTTPublisher call to mosquitto_new failed!");
    }

    // Apparently we have to do this if we use multithreading, which we do...
    // Or not? Apparently mosquitto_loop_start will call this by itself?
    // mosquitto_threaded_set(mosq, true);

    mosquitto_connect_callback_set(mosq, global_publisher_on_connect);
    mosquitto_disconnect_callback_set(mosq, global_publisher_on_disconnect);
    mosquitto_publish_callback_set(mosq, global_on_publish);

    int delay_set_rc = mosquitto_reconnect_delay_set(mosq, 1, 10, true);
    if (delay_set_rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(mosq);
        mosq = nullptr;
        throw std::runtime_error("MQTTPublisher Call to mosquitto_reconnect_delay_set failed, err=" + std::to_string(delay_set_rc) + " " + mosquitto_strerror(delay_set_rc));
    }

    int connect_rc = mosquitto_connect(mosq, broker_address.c_str(), broker_port, keepalive_seconds);
    if (connect_rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(mosq);
        mosq = nullptr;
        throw std::runtime_error("MQTTPublisher Call to mosquitto_connect(" + broker_address + ":" + std::to_string(broker_port) + ") failed, err=" + std::to_string(connect_rc) + " " + mosquitto_strerror(connect_rc));
    }

    int loop_start_rc = mosquitto_loop_start(mosq);
    if (loop_start_rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(mosq);
        mosq = nullptr;
        throw std::runtime_error("MQTTPublisher Call to mosquitto_loop_start failed, err=" + std::to_string(loop_start_rc) + " " + mosquitto_strerror(loop_start_rc));
    }
}

void MQTTPublisher::on_connect(struct mosquitto* m, int reason_code)
{
	// Print out the connection result. mosquitto_connack_string() produces an
	// appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	// clients is mosquitto_reason_string().

    if (debug) {
        cout << "MQTTPublisher on_connect: " << mosquitto_connack_string(reason_code) << std::endl;
    }

    if (reason_code != 0) {
		// If the connection fails for any reason, we don't want to keep on
		// retrying in this example, so disconnect. Without this, the client
		// will attempt to reconnect.
        //mosquitto_disconnect(m);
    }
    connected = true;

    MQTTNodeBase::on_connect(m, reason_code);
}

void MQTTPublisher::on_publish(struct mosquitto* m, int message_id)
{
    if (debug) {
        cout << "MQTTPublisher on_publish, message id = " << message_id << std::endl;
    }
}

void MQTTPublisher::receive(core::MessagePtr m)
{
    if (m == nullptr) {
        signal(nullptr);
        return;
    }

    if (!connected) {
        if (debug) {
            cout << "MQTTPublisher is NOT connected, so will not publish " << m->get_raw_size() << " bytes on \"" << topic_name << "\" with qos:" << qos << std::endl;
        }
        signal(m);
        return;
    }

    if (debug) {
        cout << "MQTTPublisher about to publish " << m->get_raw_size() << " bytes on \"" << topic_name << "\" with qos:" << qos << std::endl;
    }

    // Publish the message
	// mosq - our client instance
	// *mid = NULL - we don't want to know what the message id for this message is
	// topic = "example/temperature" - the topic on which this message will be published
	// payloadlen = strlen(payload) - the length of our payload in bytes
	// payload - the actual payload
	// qos - publish with QoS 2 (0=at most once, 1=at least once, 2=exactly once) for this example
	// retain - use the retained message feature for this message. the documentation states that
    // this controls whether the last message is broadcast to
    // slow joiners. I used to believe that retain=false causes a
    // mem leak, but I think I'm mistaken. There's definitely mem leaks here
    // and on the server.

    int resulting_message_id = 0;
    int publish_rc = mosquitto_publish(
        this->mosq,
        (int*)(&resulting_message_id),
        (const char*)(this->topic_name.c_str()),
        (int)(m->get_raw_size()),
        (const void*)(m->get_raw_data()),
        this->qos,
        this->retained);

	if (publish_rc != MOSQ_ERR_SUCCESS) {
        cout << "MQTTPublisher mosquitto_publish failed with " << publish_rc << " " << mosquitto_strerror(publish_rc) << " on topic name \"" << this->topic_name << "\"" << std::endl;
        // Do we throw? abort? nothing?
	}

    if (debug) {
        cout << "MQTTPublisher published with message id = " << resulting_message_id << " on topic name \"" << this->topic_name << "\"" << std::endl;
    }

    // propagate
    signal(m);
}


// -- MQTTSubscriber --

MQTTSubscriber::MQTTSubscriber(
    MQTTContextPtr mqtt_context,
    const string& broker_address,
    int broker_port,
    const string& topic_name,
    const string& name,
    int keepalive_seconds,
    int qos,
    int loop_timeout_milliseconds,
    bool debug):
        core::RunnableNode(name),
        MQTTNodeBase(mqtt_context, broker_address, broker_port, topic_name, keepalive_seconds, qos, debug),
        loop_timeout_milliseconds(loop_timeout_milliseconds),
        subscribed(false)
{
    // Construct an mqtt session.
    const char * client_id = NULL;
    bool clean_session = true;
    mosq = mosquitto_new(client_id, clean_session, this);
    if (mosq == nullptr) {
        throw std::runtime_error("MQTTSubscriber call to mosquitto_new failed!");
    }

    // Apparently we have to do this if we use multithreading, which we do...
    // Or not? Apparently mosquitto_loop_start will call this by itself?
    // mosquitto_threaded_set(mosq, true);

    mosquitto_connect_callback_set(mosq, global_subscriber_on_connect);
    mosquitto_disconnect_callback_set(mosq, global_subscriber_on_disconnect);
	mosquitto_subscribe_callback_set(mosq, global_on_subscribe);
	mosquitto_message_callback_set(mosq, global_on_receive);

    int connect_rc = mosquitto_connect(mosq, broker_address.c_str(), broker_port, keepalive_seconds);
    if (connect_rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(mosq);
        mosq = nullptr;
        throw std::runtime_error("MQTTSubscriber Call to mosquitto_connect(" + broker_address + ":" + std::to_string(broker_port) + ") failed, err=" + std::to_string(connect_rc));
    }
}

void MQTTSubscriber::on_connect(struct mosquitto* m, int reason_code)
{
	// Print out the connection result. mosquitto_connack_string() produces an
	// appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	// clients is mosquitto_reason_string().

    if (debug) {
        cout << "MQTTSubscriber on_connect: " << mosquitto_connack_string(reason_code) << std::endl;
    }

    if (reason_code != 0) {
		// If the connection fails for any reason, we don't want to keep on
		// retrying in this example, so disconnect. Without this, the client
		// will attempt to reconnect.
        //mosquitto_disconnect(m);
    }

	// Making subscriptions in the on_connect() callback means that if the
	// connection drops and is automatically resumed by the client, then the
	// subscriptions will be recreated when the client reconnects.
	int subscribe_rc = mosquitto_subscribe(mosq, nullptr, topic_name.c_str(), qos);
	if (subscribe_rc != MOSQ_ERR_SUCCESS){
		// We might as well disconnect if we were unable to subscribe
		mosquitto_disconnect(mosq);
        throw std::runtime_error("MQTTSubscriber Error subscribing to " + topic_name + ": " + mosquitto_strerror(subscribe_rc));
	}

    connected = true;

    MQTTNodeBase::on_connect(m, reason_code);
}

void MQTTSubscriber::on_subscribe(struct mosquitto *mosq, int mid, int qos_count, const int *granted_qos)
{
    if (debug) {
        cout << "MQTTSubscriber on_subscribe mid:" << mid << " qos_count:" << qos_count << " granted_qos:" << *granted_qos << std::endl;
    }

    subscribed = true;
}

void MQTTSubscriber::on_receive(struct mosquitto *mosq, const struct mosquitto_message *msg)
{
    // What a mosquitto_message looks like:
    // struct mosquitto_message{
    //         int mid;
    //         char *topic;
    //         void *payload;
    //         int payloadlen;
    //         int qos;
    //         bool retain;
    // };

    if (debug) {
        cout << "MQTTSubscriber::on_receive mid: " << msg->mid << " topic:" << msg->topic << " len:" << msg->payloadlen << " qos:" << msg->qos << " retain:" << msg->retain << std::endl;
    }

    // only if we have any observers do we bother signalling.
    if (this->has_observers()) {

        // the mosquitto thread will, ultimately, delete msg.
        // This means that to get the ability to hold on to
        // messages via shared pointer, we have to create
        // a copy of the data, and delete it when the backing store
        // goes out of scope. Otherwise, if the message is accessed
        // after mosquitto_message* msg is deleted, we'll get
        // a segfault.
        int data_length = msg->payloadlen;
        uint8_t* data_orig = (uint8_t*)msg->payload;
        uint8_t* data = new uint8_t[data_length];
        memcpy(data, data_orig, data_length);

        // get a payload object that will, ultimately, delete the body_message
        auto payload = std::make_shared<core::MessageBackingStoreNew>(data, data_length);

        // construct and return the message
        auto m = std::make_shared<core::Message>(payload);

        // signal my observers
        this->signal(m);
    }
}

void MQTTSubscriber::child_thread_fn()
{
    while (!this->stop_requested()) {
        mosquitto_loop(mosq, loop_timeout_milliseconds, 1);
    }
}

} // namespace transportmqtt
} // namespace roboflex
