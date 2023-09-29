#include <iostream>
#include <xtensor/xrandom.hpp>
#include "roboflex/core/core_messages/core_messages.h"
#include "roboflex/core/core_nodes/core_nodes.h"
#include "roboflex/core/util/utils.h"
#include "roboflex/visualization/visualization.h"

using namespace roboflex;

int main()
{
    auto frequency_generator = nodes::FrequencyGenerator(30.0);

    auto random_tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<uint8_t, 3> d = xt::random::randint({480, 640, 3}, 0, 255);
        return core::TensorMessage<uint8_t, 3>::Ptr(d);
    });

    auto visualizer = visualization::RGBImageTV(24.0, 640, 480, "t");

    frequency_generator > random_tensor_creator > visualizer;

    frequency_generator.start();
    visualizer.start();

    sleep_ms(5000);
    
    frequency_generator.stop();
    visualizer.stop();

    std::cout << "DONE" << std::endl;
    return 0;
}