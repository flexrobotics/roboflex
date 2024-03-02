#include <iostream>
#include "roboflex_core/core_messages/core_messages.h"
#include "roboflex_core/core_nodes/core_nodes.h"
#include "roboflex_core/util/utils.h"

using namespace roboflex;

void test_dimension_1()
{
    std::cout << std::endl << "TESTING DIMENSION 1" << std::endl;

    // The first node will signal at 2 hz
    auto frequency_generator = nodes::FrequencyGenerator(2.0);

    // The second node will create a tensor of ones, with shape (2, 3), 
    // and multiply it by the message counter. It will use a lambda function
    // to construct a MapFun node.
    auto tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<double, 1> d = xt::ones<double>({3}) * m->message_counter();
        return core::TensorMessage<double, 1>::Ptr(d);
    });

    // The third node will print the message.
    auto message_printer = nodes::MessagePrinter("TENSOR IS:");

    // Instantiate the tensor buffer node.
    auto tensor_buffer = nodes::TensorRightBuffer<double>({10}, "t");

    // The fourth node will print the buffer.
    auto tensor_buffer_printer = nodes::CallbackFun([](core::MessagePtr m) {
        auto b = core::TensorBufferMessage<double>(*m);
        auto d = b.buffer();
        auto c = b.count();
        std::cout << "FINAL BUFFER (" << c << ") IS:" << std::endl << d << std::endl;
    });

    // connect the nodes; frequency generator signals to tensor_creator, which signals...
    frequency_generator > tensor_creator > message_printer > tensor_buffer > tensor_buffer_printer;

    // start the frequency generator (the only node that needs starting).
    frequency_generator.start();

    sleep_ms(3000);
    
    frequency_generator.stop();

    std::cout << "DONE" << std::endl;
}

void test_dimension_2()
{
    std::cout << std::endl << "TESTING DIMENSION 3" << std::endl;

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
    auto message_printer = nodes::MessagePrinter("TENSOR IS:");

    // Instantiate the tensor buffer node.
    auto tensor_buffer = nodes::TensorRightBuffer<double>({2, 10}, "t");

    // The fourth node will print the buffer.
    auto tensor_buffer_printer = nodes::CallbackFun([](core::MessagePtr m) {
        auto b = core::TensorBufferMessage<double>(*m);
        auto d = b.buffer();
        auto c = b.count();
        std::cout << "BUFFER (" << c << ") IS:" << std::endl << d << std::endl;
    });

    // connect the nodes; frequency generator signals to tensor_creator, which signals...
    frequency_generator > tensor_creator > message_printer > tensor_buffer > tensor_buffer_printer;

    // start the frequency generator (the only node that needs starting).
    frequency_generator.start();

    sleep_ms(3000);
    
    frequency_generator.stop();

    std::cout << "DONE" << std::endl;
}

void test_dimension_3()
{
    std::cout << std::endl << "TESTING DIMENSION 3" << std::endl;

    // The first node will signal at 2 hz
    auto frequency_generator = nodes::FrequencyGenerator(2.0);

    // The second node will create a tensor of ones, with shape (2, 3), 
    // and multiply it by the message counter. It will use a lambda function
    // to construct a MapFun node.
    auto tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<double, 3> d = xt::ones<double>({2, 3, 3}) * m->message_counter();
        return core::TensorMessage<double, 3>::Ptr(d);
    });

    // The third node will print the message.
    auto message_printer = nodes::MessagePrinter("TENSOR IS:");

    // Instantiate the tensor buffer node.
    auto tensor_buffer = nodes::TensorRightBuffer<double>({2, 3, 10}, "t");

    // The fourth node will print the buffer.
    auto tensor_buffer_printer = nodes::CallbackFun([&tensor_buffer](core::MessagePtr m) {
        auto b = core::TensorBufferMessage<double>(*m);
        auto d = b.buffer();
        auto c = b.count();
        tensor_buffer.chop(5, -2.0);
        std::cout << "BUFFER (" << c << ") IS:" << std::endl << d << std::endl;
    });

    // connect the nodes; frequency generator signals to tensor_creator, which signals...
    frequency_generator > tensor_creator > message_printer > tensor_buffer > tensor_buffer_printer;

    // start the frequency generator (the only node that needs starting).
    frequency_generator.start();

    sleep_ms(3000);
    
    frequency_generator.stop();

    std::cout << "DONE" << std::endl;
}

int main() 
{
    test_dimension_1();
    test_dimension_2();
    test_dimension_3();
    return 0;
}