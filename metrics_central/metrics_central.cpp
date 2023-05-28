#include <iostream>
#include <boost/program_options.hpp>
#include "transport/mqtt/mqtt_nodes.h"
#include "metrics_central_impl.h"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {

    std::string mqtt_broker_address;
    int mqtt_broker_port;
    std::string mqtt_metrics_topic;

    // Get CLI options
    po::options_description desc("Metrics Central Options");
    desc.add_options()
        ("help,h", "produce help message")
        ("mqtt_broker_address,a", po::value<std::string>(&mqtt_broker_address)->default_value("127.0.0.1"), "MQTT broker address.")
        ("mqtt_broker_port,p", po::value<int>(&mqtt_broker_port)->default_value(1883), "MQTT broker port.")
        ("mqtt_metrics_topic,t", po::value<std::string>(&mqtt_metrics_topic)->default_value("roboflexmetrics"), "MQTT metrics topic.")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cerr << desc << "\n";
        return 1;
    }

    std::cerr << "STARTING METRICS CENTRAL, LISTENING TO TOPIC "
              << "\"" << mqtt_metrics_topic << "\" AT "
              << mqtt_broker_address << ":"
              << mqtt_broker_port
              << std::endl;

    try {

        // Set up the mqtt subscriber that will receive metrics messages from a broker.
        auto context = roboflex::transportmqtt::MakeMQTTContext();
        int keepalive_seconds = 60;
        int qos = 0;
        int loop_timeout_milliseconds = 100;
        bool debug = false;
        std::string node_name = "MQTTMetricsSubscriber";
    
        auto subscriber = roboflex::transportmqtt::MQTTSubscriber(
            context, mqtt_broker_address, mqtt_broker_port, mqtt_metrics_topic, node_name, keepalive_seconds, qos, loop_timeout_milliseconds, debug);

        subscriber.start();

        std::cerr << "RUNNING..." << std::endl;

        // Run the metrics viewer in a window.
        std::string window_title = "Metrics Central: " +
            mqtt_metrics_topic + " on " + mqtt_broker_address +
            ":" + std::to_string(mqtt_broker_port);

        // Will block until the window is closed.
        int retval = run_metrics_viewer(subscriber, window_title);

        subscriber.stop();

        return retval;

    } catch (...) {
        std::cerr << "\n\nHEY, COULD NOT CONNECT TO AN MQTT BROKER.\n"
                  << "IS ONE RUNNING AT " << mqtt_broker_address << ":"
                  << mqtt_broker_port << "?\n"
                  << "You might want to install one - try googling for "
                  << "how to install mosquitto, a popular mqtt broker.\n\n" << std::endl;
        throw;
    }
}
