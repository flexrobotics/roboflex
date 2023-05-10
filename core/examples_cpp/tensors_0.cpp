#include <iostream>
#include "core/core_messages/core_messages.h"
#include "core/core_nodes/frequency_generator.h"
#include "core/core_nodes/map_fun.h"
#include "core/core_nodes/callback_fun.h"
#include "core/core_nodes/message_printer.h"
#include "core/util/utils.h"

using namespace roboflex::core;
using namespace roboflex::nodes;

int main() {
    FrequencyGenerator frequency_generator(2.0);

    auto tensor_creator = MapFun([](MessagePtr m) {
        xt::xtensor<double, 2> d = xt::ones<double>({2, 3}) * m->message_counter();
        return TensorMessage<double, 2>::Ptr(d);
    });

    auto message_printer = MessagePrinter("MESSAGE IS:");

    auto tensor_printer = CallbackFun([](MessagePtr m) {
        xt::xtensor<double, 2> d = TensorMessage<double, 2>(*m).value();
        std::cout << "TENSOR IS:" << std::endl << d << std::endl;
    });

    frequency_generator > tensor_creator > message_printer > tensor_printer;
    frequency_generator.start();
    sleep_ms(3000);
    frequency_generator.stop();

    std::cout << "DONE" << std::endl;
    return 0;
}