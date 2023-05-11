# roboflex.transport.zmq examples

### 1. [pub_sub_0.cpp](pub_sub_0.cpp): Tests sending TensorMessage (which uses xtensor) over ZMQ. Should look exactly the same as core/examples/tensor_0.cpp, but with a ZMQ pub-sub step.

        bazel run -c opt //transport/zmq/examples:pub_sub_0