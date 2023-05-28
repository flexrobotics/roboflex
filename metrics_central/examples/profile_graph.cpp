#include <iostream>
#include "core/core_messages/core_messages.h"
#include "core/core_nodes/core_nodes.h"
#include "core/util/utils.h"
#include "metrics_central/profiler.h"

using namespace roboflex;

int main() 
{
    auto frequency_generator = nodes::FrequencyGenerator(100.0);

    auto tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = xt::ones<double>({100, 1000}) * m->message_counter();
        return core::TensorMessage<double, 2>::Ptr(d);
    });

    auto tensor_printer = nodes::CallbackFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = core::TensorMessage<double, 2>(*m).value();
        std::cout << "TENSOR IS:" << std::endl << d << std::endl;
    });

    frequency_generator > tensor_creator > tensor_printer;

    profiling::Profiler profiler;
    profiler > frequency_generator;

    std::cout << "BEFORE INSTRUMENTATION:\n" << profiler.graph_to_string() << std::endl;

    profiler.start(true);

    std::cout << "AFTER INSTRUMENTATION:\n" << profiler.graph_to_string() << std::endl;

    sleep_ms(2000000);

    profiler.stop();

    std::cout << "AFTER DE-INSTRUMENTATION:\n" << profiler.graph_to_string() << std::endl;

    return 0;
}