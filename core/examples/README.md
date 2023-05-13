# roboflex.core examples


## 1. **base_0** 

A basic graph. Shows custom message, custom Producer Node, custom Consumer Node.

c++: [cpp/basic_0.cpp](cpp/basic_0.cpp)
                
    bazel run -c opt //core/examples/cpp:basic_0

python [python/basic_0.py](cpp/basic_0.cpp)

    bazel run -c opt //core/examples/python:basic_0


## 2. **tensors_0**

A basic graph that sends around tensors. Shows FrequencyGenerator, MapFun, CallbackFun, TensorMessage, MessagePrinter.

c++: [cpp/tensors_0.cpp](cpp/tensors_0.cpp)
                
    bazel run -c opt //core/examples/cpp:basic_0

python [python/tensors_0.py](cpp/tensors_0.cpp)

    bazel run -c opt //core/examples/python:tensors_0

