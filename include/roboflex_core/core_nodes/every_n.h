#ifndef ROBOFLEX_EVERY_N__H
#define ROBOFLEX_EVERY_N__H

#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * A Node that signals every n'th received message.
 */
class EveryN: public Node {
public:
    EveryN(int n, const std::string& name = "EveryN"):
        Node(name),
        n(n),
        count(0) {}

    void receive(MessagePtr m) override {
        if (count == 0) {
            signal(m);
        }
        count += 1;
        if (count == n) {
            count = 0;
        }
    }

    int n;
    int count;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_EVERY_N__H
