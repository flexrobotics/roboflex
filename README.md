# roboflex

Roboflex is a c++/python library for distributed robotics. Roboflex is designed for:

1. Trivial installation: the core library has no dependencies other than c++ 20.
2. Ease of use: no non-programming-language files and configurations.
3. Performance: supports 0-copy xtensors and eigen in c++.
4. Python interaction: easily scriptable with python. Still performant.
5. You control main(): Roboflex is a library, not a framework.

Roboflex supports any wire transport, starting with ZMQ and MQTT.

## Messaging

The core of roboflex is the message format. We believe that dynamic, self-describing messages are extremely important for truly distributed development. To that end, we choose FlexBuffers as the message format, with a few additions. See [MESSAGEFORMAT.md](core/MESSAGEFORMAT.md).

## Building

See [BUILDING.md](BUILDING.md) for how to build parts or all of roboflex.

## core vs other

Roboflex.core takes care of Message and Node classes, eigen/xtensor serialization, flexbuffer creation and reading, and python compatibility. Everything outside of roboflex.core is a sensor, actuator, robot, or utility node - roboflex aims to make it easy to create and support devices and tools.

## Real quick

Run [core/examples/tensors_0.cpp](core/examples/tensors_0.cpp) to see a quick example, in c++, of running a producer sending xtensor messages to a consumer.

    bazel run -c opt //core/examples:tensors_0