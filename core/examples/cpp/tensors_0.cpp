#include <iostream>
#include "core/core_messages/core_messages.h"
#include "core/core_nodes/core_nodes.h"
#include "core/util/utils.h"

using namespace roboflex;

int main() 
{
    auto frequency_generator = nodes::FrequencyGenerator(2.0);

    auto tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = xt::ones<double>({2, 3}) * m->message_counter();
        return core::TensorMessage<double, 2>::Ptr(d);
    });

    auto message_printer = nodes::MessagePrinter("MESSAGE IS:");

    auto tensor_printer = nodes::CallbackFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = core::TensorMessage<double, 2>(*m).value();
        std::cout << "TENSOR IS:" << std::endl << d << std::endl;
    });

    // connect the nodes; frequency generator signals to tensor_creator, which signals...
    frequency_generator > tensor_creator > message_printer > tensor_printer;

    frequency_generator.start();

    sleep_ms(3000);
    
    frequency_generator.stop();

    std::cout << "DONE" << std::endl;
    return 0;
}