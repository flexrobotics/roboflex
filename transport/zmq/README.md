# robocore.zmq

Robocore support for the ZMQ transport.

    any node -> ZMQPublisher ==THEINTERNET==> ZMQSubscriber -> any node

See https://zeromq.org/ for details.

Using ZMQ, nodes can connect to other nodes, running in different threads, different processes, or different computers, with a publisher-subscriber pattern. robocore.zmq supports:

    "inproc" transport -> between threads within same process
    "ipc" transport -> between processes on same computer
    "tcp" transport -> between processes on different computers


## System Dependencies

    apt-get install libzmq3-dev

## Build

    bazel build -c opt //transport/zmq/...

## Run Examples

    bazel run -c opt //transport/zmq/examples_cpp:pub_sub_0
