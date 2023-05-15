# roboflex examples

These are examples of roboflex that use more than one module.

## 1. **webcam_tv**

Webcam television! If you have a libuvc-(usb)-enabled camera, and have installed the system dependencies for roboflex.webcam_uvc and roboflex.visualization, then you should be able to run this. This script runs a webcam_uvc sensor (requires a usb webcam), and a visualizer.

c++: [webcam_tv/webcam_tv_cpp.cpp](webcam_tv/webcam_tv_cpp.cpp)

    $ bazel run -c opt //examples/webcam_tv:webcam_tv_cpp

python: [webcam_tv/webcam_tv_py.py](webcam_tv/webcam_tv_py.py)

    $ bazel run -c opt //examples/webcam_tv:webcam_tv_py


## 2. **webcam_tv_distributed**

Distributed webcam television! If you also have the system dependencies for roboflex.transport.zmq installed, as well as python, you should be able to run this. It runs a webcam_uvc sensor (requires a usb webcam), broadcasts the resulting images over zmq, subscribes to zmq, and displays the results, all in python.

python: [webcam_tv_distributed/webcam_tv_distributed_zmq.py](webcam_tv_distributed/webcam_tv_distributed_zmq.py)

    $ bazel run -c opt //examples/webcam_tv_distributed:webcam_tv_distributed_zmq

<!-- .. and if you also have the system dependencies for roboflex.transport.mqtt installed, AND are running some mqtt broker at 127.0.0.1:1883 (can be changed in the code), you should be able to do this:

    $ bazel run -c opt //examples/webcam_tv_distributed:webcam_tv_distributed_mqtt -->

