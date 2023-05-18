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

## Modules and Directory Structure

Roboflex.core takes care of Message and Node classes, eigen/xtensor serialization, flexbuffer creation and reading, and python compatibility. Everything outside of roboflex.core is a sensor, actuator, robot, or utility node - roboflex aims to make it easy to create and support devices and tools. These are the folders/modules under roboflex, and what they do:

* [core](core/): Serialization, Node and Message based classes, etc.

Utilities:
* [transport](transport/): Folder containing sub-modules that perform wire transport, such as nodes using ZMQ and MQTT.
* [visualization](visualization/): Various visualizers using Simple Directmedia Layer.

Devices:
* [dynamixel](dynamixel/): Support for dynamixel motors.
* [webcam_uvc](webcam_uvc/): Support for usb-connect, uvc-compatible webcams.

Compound examples:
* [examples](examples/): Compound examples that use more than one module.


## Real quick

Run [core/examples/cpp/tensors_0.cpp](core/examples/cpp/tensors_0.cpp) to see a quick example, in c++, of running a producer sending xtensor messages to a consumer in a single thread.

    bazel run -c opt //core/examples/cpp:tensors_0