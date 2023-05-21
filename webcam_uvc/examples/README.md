# webcam_uvc examples


## 1. **run_webcam** 

Constructs a WebcamSensor, starts it, prints resulting messages. You must have a UVC-compatible webcam attached (usually via USB). Doesn't display the result - see examples for a version of this program that also visualizes the output.

c++: [run_webcam_cpp.cpp](run_webcam_cpp.cpp)
                
    bazel run -c opt //webcam_uvc/examples:run_webcam_cpp

python: [run_webcam_py.py](run_webcam_py.py)

    bazel run -c opt //webcam_uvc/examples:run_webcam_py
