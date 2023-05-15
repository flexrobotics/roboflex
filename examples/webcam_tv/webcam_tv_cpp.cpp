#include <iostream>
#include <unistd.h>
#include <libuvc/libuvc.h>
#include "core/core.h"
#include "webcam_uvc/webcam_uvc.h"
#include "visualization/visualization.h"

using namespace roboflex;

int main() 
{
    uint16_t WIDTH = 800;
    uint16_t HEIGHT = 600;
    uint16_t FPS = 60;
    uvc_frame_format FORMAT = uvc_frame_format::UVC_FRAME_FORMAT_MJPEG;

    std::cout << "DEVICES: " << std::endl << webcam_uvc::get_device_list_string();
    
    auto sensor = webcam_uvc::WebcamSensor(
        WIDTH,
        HEIGHT,
        FPS,
        1,
        FORMAT,
        true);
    sensor.print_device_info();

    auto viewer = visualization::RGBImageTV(
        24.0, 
        WIDTH, 
        HEIGHT, 
        "rgb",
        {20,20},
        true,
        false);

    sensor > viewer;

    sensor.start();
    viewer.start();

    sleep(60);

    sensor.stop();
    viewer.stop();

    return 0;
}