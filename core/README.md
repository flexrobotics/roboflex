# roboflex.core

At its core, roboflex is a library to make Nodes that create, signal, and receive Messages to and from other nodes, in a distributed manner.

![](roboflex_nodes_messages_2.png)

Roboflex.core defines what a Message is and what a Node is. It provides serialization services for eigen and xtensor. It provides a small library of core message types and useful Node sub-classes. The core only supports sending messages via direct function call to other nodes; the nodes in transport/ (zmq and mqtt so far) support sending messages from one thread to another, from one process to another, and from one computer to another via multiple methods.

## Basic Types

### Message

A roboflex Message is defined as:
1. An 8-byte header. The first four bytes are the letters 'RFLX', and the next four bytes
are a uint32, which is the size of the message in bytes (including the header).
2. A data portion, encoded in flexbuffers.h. Please see [MESSAGEFORMAT.md](MESSAGEFORMAT.md).
3. roboflex.core provides the Message class to facilitate this, as well as meta data and 0-copy functionality. This class is designed to be inherited. Please see [core_messages.h](core_messages/core_messages.h) for examples.

### Node

A roboflex Node represents a basic unit of computation. Nodes can connect to other nodes, can signal Messages, and can receive them. RunnableNode inherits from Node, and adds the ability to run a function in a thread. Both Node and Runnable are designed to be inherited: in order to perform custom logic on message reception, custom nodes should inherit from Node and override receive. In order to run in a thread, custom nodes should inherit RunnableNode and override child_thread_fn.


## Building

bazel build -c opt //core:roboflex_core


## Examples

see [examples/README.md](examples/README.md)