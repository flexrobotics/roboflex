# examples.webcam_fastsam

This example demonstrates running a webcam through the FastSAM segmentation algorithm, and uses the FastSAM code from https://github.com/CASIA-IVA-Lab/FastSAM as a submodule, so be sure to:

    git submodule init
    git submodule update

Then download either FastSAM-s.pt or FastSAM-x.pt (or both), as described in the FastSAM website (above), and place into examples/webcam_fastsam/models.

NOTE! FastSAM sometimes crashes if an input image has nothing to segment, or too much motion blur.


Run like so:

1. Create a conda or pyvenv environment using the requirements.txt file, and activate it.

Then:
        
        bazel run -c opt //examples/webcam_fastsam:webcam_fastsam

This examples has these system dependencies:

1. webcam_uvc:

    apt-get install libusb-1.0-0-dev
    apt-get install libjpeg-dev

2. visualization

    sudo apt-get install libsdl2-dev

3. mqtt

    apt-get install mosquitto
    apt-get install libmosquitto-dev