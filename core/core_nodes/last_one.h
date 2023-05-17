#ifndef ROBOFLEX_NODES_LAST_ONE__H
#define ROBOFLEX_NODES_LAST_ONE__H

#include <atomic>
#include "core/node.h"

using std::atomic, std::string;

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * A Node that remembers the last message.
 */
class LastOne: public Node {
public:
    LastOne(const string& name = "LastOne"):
        Node(name), last_message(nullptr) {}

    MessagePtr get_last_message() { return last_message; }

    void receive(MessagePtr m) override {
        last_message = m;
        this->signal(m);
    }

protected:

    atomic<MessagePtr> last_message;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_NODES_LAST_ONE__H
