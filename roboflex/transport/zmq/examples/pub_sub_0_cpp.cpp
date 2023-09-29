#include <iostream>
#include "roboflex/core/core_messages/core_messages.h"
#include "roboflex/core/core_nodes/core_nodes.h"
#include "roboflex/core/util/utils.h"
#include "roboflex/transport/zmq/zmq_nodes.h"

using namespace roboflex;

int main()
{
    nodes::FrequencyGenerator frequency_generator(2.0);

    auto tensor_creator = nodes::MapFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = xt::ones<double>({2, 3}) * m->message_counter();
        return core::TensorMessage<double, 2>::Ptr(d);
    });

    auto zmq_context = transportzmq::MakeZMQContext();
    auto zmq_pub = transportzmq::ZMQPublisher(zmq_context, "inproc://testit");
    auto zmq_sub = transportzmq::ZMQSubscriber(zmq_context, "inproc://testit");

    auto message_printer1 = nodes::MessagePrinter("SENDING MESSAGE:");
    auto message_printer2 = nodes::MessagePrinter("RECEIVED MESSAGE:");

    auto tensor_printer = nodes::CallbackFun([](core::MessagePtr m) {
        xt::xtensor<double, 2> d = core::TensorMessage<double, 2>(*m).value();
        std::cout << "RECEIVED TENSOR:" << std::endl << d << std::endl;
    });

    // connect the nodes; frequency generator signals to tensor_creator, which signals...
    frequency_generator > tensor_creator > message_printer1 > zmq_pub;
    zmq_sub > message_printer2 > tensor_printer;

    frequency_generator.start();
    zmq_sub.start();

    sleep_ms(3000);
    
    frequency_generator.stop();
    zmq_sub.stop();

    std::cout << "DONE" << std::endl;
    return 0;
}