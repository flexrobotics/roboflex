# roboflex.transport.zmq

Roboflex support for the ZMQ transport.

    any node -> ZMQPublisher ==THEINTERNET==> ZMQSubscriber -> any node

See https://zeromq.org/ for details.

Using ZMQ, nodes can connect to other nodes, running in different threads, different processes, or different computers, with a publisher-subscriber pattern. roboflex.transport.zmq supports:

    "inproc" transport -> between threads within same process
    "ipc" transport -> between processes on same computer
    "tcp" transport -> between processes on different computers


## System Dependencies

    apt-get install libzmq3-dev

## Build

    bazel build -c opt //transport/zmq/...

## Run Examples (see [examples](examples))

    bazel run -c opt //transport/zmq/examples:pub_sub_0_cpp
    bazel run -c opt //transport/zmq/examples:pub_sub_0_py

## TODO: class documentation in c++ and python