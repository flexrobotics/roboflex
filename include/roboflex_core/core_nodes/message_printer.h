#ifndef ROBOFLEX_MESSAGE_PRINTER__H
#define ROBOFLEX_MESSAGE_PRINTER__H

#include <iostream>
#include "roboflex_core/node.h"

namespace roboflex::nodes {

/**
 * A pass-through node that prints messages to std::cout along the way.
 */
class MessagePrinter: public core::Node {
public:
    MessagePrinter(const std::string &name = "MessagePrinter"): Node(name) {}

    void receive(core::MessagePtr m) override {
        std::cout << get_name() << " " << m->to_string() << std::endl;
        signal(m);
    }
};

} // namespace roboflex::nodes 

#endif // ROBOFLEX_MESSAGE_PRINTER__H
