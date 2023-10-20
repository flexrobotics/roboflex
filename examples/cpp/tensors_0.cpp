#include <iostream>
#include "roboflex_core/core_messages/core_messages.h"
#include "roboflex_core/core_nodes/core_nodes.h"
#include "roboflex_core/util/utils.h"

using namespace roboflex;

int main() 
{
    // The first node will signal at 2 hz
    auto frequency_generator = nodes::FrequencyGenerator(2.0);

    // The second node will create a tensor of ones, with shape (2, 3), 
    // and multiply it by the message counter. It will use a lambda function
    // to construct a MapFun node.
    auto tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = xt::ones<double>({2, 3}) * m->message_counter();
        return core::TensorMessage<double, 2>::Ptr(d);
    });

    // The third node will print the message.
    auto message_printer = nodes::MessagePrinter("MESSAGE IS:");

    // The fourth node will print the tensor.
    auto tensor_printer = nodes::CallbackFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = core::TensorMessage<double, 2>(*m).value();
        std::cout << "TENSOR IS:" << std::endl << d << std::endl;
    });

    // connect the nodes; frequency generator signals to tensor_creator, which signals...
    frequency_generator > tensor_creator > message_printer > tensor_printer;

    // start the frequency generator (the only node that needs starting).
    frequency_generator.start();

    sleep_ms(3000);
    
    frequency_generator.stop();

    std::cout << "DONE" << std::endl;
    return 0;
}