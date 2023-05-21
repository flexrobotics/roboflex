#include <iostream>
#include "core/core_nodes/core_nodes.h"
#include "core/util/utils.h"
#include "realsense/realsense.h"

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

    auto message_printer = nodes::MessagePrinter("MESSAGE IS:");

    sensor > message_printer;

    sensor.start();
    sleep_ms(5000);
    sensor.stop();

    return 0;
}
