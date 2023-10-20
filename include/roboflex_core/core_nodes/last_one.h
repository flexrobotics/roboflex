#ifndef ROBOFLEX_NODES_LAST_ONE__H
#define ROBOFLEX_NODES_LAST_ONE__H

#include <atomic>
#include <mutex>
#include "roboflex_core/node.h"

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
        Node(name) {}

    MessagePtr get_last_message() { 
        std::lock_guard<std::recursive_mutex> lock(last_message_mutex); 
        return last_message; 
    }

    void receive(MessagePtr m) override {
        {
            std::lock_guard<std::recursive_mutex> lock(last_message_mutex);
            last_message = m;
        }
        this->signal(m);
    }

protected:

    // Requires gcc 12.1... do we force upgrade to 12.1?
    // atomic<MessagePtr> last_message;
    mutable std::recursive_mutex last_message_mutex;
    MessagePtr last_message = nullptr;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_NODES_LAST_ONE__H
