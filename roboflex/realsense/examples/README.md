# realsense examples


## 1. **run_realsense** 

Constructs a RealsenseSensor, starts it, prints resulting messages. Doesn't display the result - see examples for a version of this program that also visualizes the output.

c++: [run_realsense_cpp.cpp](run_realsense_cpp.cpp)
                
    bazel run -c opt //roboflex/realsense/examples:run_realsense_cpp

python: [run_realsense_py.py](run_realsense_py.py)

    bazel run -c opt //roboflex/realsense/examples:run_realsense_py
