#include <iostream>
#include <unistd.h>
#include "core/core.h"
#include "realsense/realsense.h"
#include "visualization/visualization.h"

using namespace roboflex;

int main() 
{
    realsense::RealsenseConfig config;
    config.align_to = realsense::CameraType::RGB;
    config.rgb_settings.fps = 30;
    config.rgb_settings.width = 640;
    config.rgb_settings.height = 480;
    config.depth_settings.fps = 30;
    config.depth_settings.width = 640;
    config.depth_settings.height = 480;
    realsense::RealsenseSensor sensor("827112072758", config);

    auto viewer = visualization::RGBImageTV(
        24.0, 
        640, 
        480, 
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