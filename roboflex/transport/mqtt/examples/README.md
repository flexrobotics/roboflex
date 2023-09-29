# roboflex.transport.mqtt examples


## 1. **pub_sub_0** 

Tests sending TensorMessage (which uses xtensor or numpy, in the case of python) over MQTT. Should look exactly the same as core/examples/tensor_0.cpp, but with an MQTT pub-sub step. Requires some MQTT broker to be running on 127.0.0.1, port 1883. Mosquitto works fine.

c++: [pub_sub_0_cpp.cpp](pub_sub_0_cpp.cpp)
                
    bazel run -c opt //roboflex/transport/mqtt/examples:pub_sub_0_cpp

python: [pub_sub_0_py.py](pub_sub_0_py.py)

    bazel run -c opt //roboflex/transport/mqtt/examples:pub_sub_0_py
