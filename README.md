# roboflex

Roboflex is a c++/python library for distributed robotics. Roboflex is designed for:

1. Trivial installation: the core library has no dependencies other than c++ 20.
2. Ease of use: no non-programming-language files and configurations.
3. Performance: supports 0-copy xtensors and eigen in c++.
4. Python interaction: easily scriptable with python. Still performant.
5. You control main(): Roboflex is a library, not a framework.

Roboflex supports any wire transport, starting with ZMQ and MQTT.

The core of roboflex is the message format. We believe that dynamic, self-describing messages are extremely important for truly decentralized development. To that end, we choose FlexBuffers as the message format, with a few additions. Please see [MESSAGEFORMAT.md](core/MESSAGEFORMAT.md).

Please see [BUILDING.md](BUILDING.md) for how to build parts or all of roboflex.

roboflex.core takes care of messaging and Node classes, as well as eigen/xtensor serialization, and python compatibility. Everything outside of roboflex.core is a sensor, actuator, robot, or utility node - roboflex aims to support a large variety of devices and tools, and make it easy to create them.
