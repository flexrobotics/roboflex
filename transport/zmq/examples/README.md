# roboflex.transport.zmq examples


## 1. **pub_sub_0** 

Tests sending TensorMessage (which uses xtensor or numpy, in the case of python) over ZMQ. Should look exactly the same as core/examples/tensor_0.cpp, but with a ZMQ pub-sub step.

c++: [pub_sub_0_cpp.cpp](pub_sub_0_cpp.cpp)
                
    bazel run -c opt //transport/zmq/examples:pub_sub_0_cpp

python: [pub_sub_0_py.py](pub_sub_0_py.py)

    bazel run -c opt //transport/zmq/examples:pub_sub_0_py
