# examples.camera_follow

This example demonstrates usage of several roboflex modules, including dynamixel, webcam_uvc, transport/mqtt, and metrics_central. In particular, it demonstrates how easy it is to perform computer vision tasks using python in roboflex.

This example creates a face-following camera using a webcam and two dynamixel motors. It uses the 'yoloface' repo (cloned from https://github.com/elyha7/yoloface into this repo) to perform face detection. It requires two dynamixel motors arranged in a pan-tilt configuration, and a uvc-compatible webcam. Rubber-banding optional.

![](pan_tilt_dynamixels.jpg)

To run:

    # Create and activate some sort of python virtual
    # environment. There is a requirements.txt file. 
    cd examples/camera_follow
    source pyvenv/bin/activate
    cd ../..

    # Run the example.
    bazel run -c opt //examples/camera_follow:camera_follow

The code will automatically publish profiling information through an MQTT broker, which you must be running. To visualize it, simultaneously run:

    bazel run -c opt //metrics_central:metrics_central


<video src='camera_follow_in_action.mp4' width=644/>

https://github.com/flexrobotics/roboflex/assets/132782/406861d0-bbd5-47ae-b10e-8b04270e153b

