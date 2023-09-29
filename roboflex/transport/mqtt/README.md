# robocore.mqtt

Robocore support for the MQTT transport.

See https://mqtt.org/ for details.

Using MQTT, nodes can connect to other nodes, even running on different computers. You must run your own MQTT broker - broker functionality is not wrapped in any way.

any node -> MQTTPublisher ==MQTT BROKER==> MQTTSubscriber -> any node

## Contents

* [Install](#install)
* [Import](#import)
* [Classes](#classes)
    * [MQTTContext](#mqttontext)
    * [MQTTNodeBase](#mqttnodebase)
    * [MQTTPublisher](#mqttpublisher)
    * [MQTTSubscriber](#mqttsubscriber)


## System Dependencies

    apt-get install mosquitto
    apt-get install libmosquitto-dev

## Build

    bazel build -c opt //roboflex/transport/mqtt/...

## Run Examples (see [examples](examples))

    bazel run -c opt //roboflex/transport/mqtt/examples:pub_sub_0_cpp
    bazel run -c opt //roboflex/transport/mqtt/examples:pub_sub_0_py

## Import

    import robocore.transport.mqtt as srm

# Classes

Robocore's support for MQTT is embodied in four classes:

1. MQTTContext, which you just have to instantiate somewhere
2. MQTTNodeBase, a base class which you don't use
3. MQTTPublisher, which can publish to an mqtt topic somewhere
4. MQTTSubscriber, which can subscribe to an mqtt topic somewhere


## MQTTContext

In order to use the other MQTT classes, you must instantiate an MQTTContext, and its lifetime must be >= the lifetime of all other MQTT node classes. You must pass an instance of this class to the constructors
of both MQTTPublisher and MQTTSubscriber.

    # instantiate like so:
    mqtt_context = rcm.MQTTContext()

## MQTTNodeBase

Do not instantiate directly. This is the base class for MQTTPublisher and MQTTSubscriber, and holds common functionality and properties.

    # the address of the broker
    mqtt_node.broker_address -> str

    # the port number of the broker
    mqtt_node.broker_port -> int

    # the number of seconds between keepalive messages
    mqtt_node.keepalive_seconds -> int

    # the topic to publish or subscribe to
    mqtt_node.topic_name -> str

    # the mqtt quality-of-service
    mqtt_node.qos -> int

    # whether to print out debug messages
    mqtt_node.debug -> bool

## MQTTPublisher

_**(inherits MQTTNodeBase)**_

Publishes any messages it receives to some topic, on some broker. When it receives an message, it publishes the binary representation on the given topic, and then propagages the message verbatim.

    mqtt_publisher = rcm.MQTTPublisher(
        mqtt_context: rcm.MQTTContext,
        broker_address: str,
        broker_port: int,
        keepalive_seconds: int,
        topic_name: str,

        # optional...
        name: str = "MQTTPublisher",
        qos: int = 0,
        retained: bool = false,
        debug: bool = false,
    )

    # additional properties:

    # See the MQTT documentation for what the retained feature does:
    # https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/
    mqtt_publisher.retained -> bool

    # If you have some message 'in hand' in some other function,
    # you can just use an MQTTPublisher to publish the message
    # directly. This is just an alias for 'signal_self' on core::Node.
    mqtt_publisher.publish({"key1": 32})


## MQTTSubscriber

_**(inherits MQTTNodeBase)**_

Suscribes to some topic from some broker. Expects only Robocore encoded messages.

    mqtt_subscriber = rcm.MQTTSubscriber(
        mqtt_context: rcm.MQTTContext,
        broker_address: str,
        broker_port: int,
        keepalive_seconds: int,
        topic_name: str,

        # optional...
        name: str = "MQTTSubscriber",
        qos: int = 0,
        loop_timeout_milliseconds: int = 100,
        debug: bool = false,
    )

    # additional properties:

    # number of milliseconds, max, to wait in the mqtt message loop,
    # before checking whether to continue or not.
    mqtt_subscriber.loop_timeout_milliseconds -> int
