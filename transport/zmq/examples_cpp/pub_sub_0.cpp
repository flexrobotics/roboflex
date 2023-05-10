#include <iostream>
#include "core/core_messages/core_messages.h"
#include "core/core_nodes/core_nodes.h"
#include "core/util/utils.h"
#include "transport/zmq/zmq_nodes.h"

using namespace roboflex::core;
using namespace roboflex::nodes;
using namespace roboflex::roboflexzmq;

int main()
{
    FrequencyGenerator frequency_generator(2.0);

    auto tensor_creator = MapFun([](MessagePtr m) {
        xt::xtensor<double, 2> d = xt::ones<double>({2, 3}) * m->message_counter();
        return TensorMessage<double, 2>::Ptr(d);
    });

    auto zmq_context = MakeZMQContext();
    auto zmq_pub = ZMQPublisher(zmq_context, "inproc://testit");
    auto zmq_sub = ZMQSubscriber(zmq_context, "inproc://testit");

    auto message_printer1 = MessagePrinter("SENDING MESSAGE:");
    auto message_printer2 = MessagePrinter("RECEIVED MESSAGE:");

    auto tensor_printer = CallbackFun([](MessagePtr m) {
        xt::xtensor<double, 2> d = TensorMessage<double, 2>(*m).value();
        std::cout << "RECEIVED TENSOR:" << std::endl << d << std::endl;
    });

    frequency_generator > tensor_creator > zmq_pub > message_printer1;
    zmq_sub > message_printer2 > tensor_printer;
    frequency_generator.start();
    zmq_sub.start();

    sleep_ms(3000);
    
    frequency_generator.stop();
    zmq_sub.stop();

    std::cout << "DONE" << std::endl;
    return 0;
}