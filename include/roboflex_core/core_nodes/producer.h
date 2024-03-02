#ifndef ROBOFLEX_PRODUCER__H
#define ROBOFLEX_PRODUCER__H

#include "roboflex_core/node.h"
#include "roboflex_core/util/event.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * Receives messages from upstream, and stores them one by one.
 * 
 * Runs in its own thread: continuously checks for new messages,
 * and if there is one, it calls produce() and signals the result.
 * Can run slower or faster than the upstream, and so might skip
 * messages if it runs slower.
 * 
 * Designed to be inherited from, and the produce() method overridden.
 * Default implementation of produce() just returns the message.
 * 
 * TODO: make it take a lambda function
 */
class Producer: public RunnableNode {
public:
    Producer(
        int timeout_milliseconds = 1000, 
        const std::string& name = "Producer"):
            RunnableNode(name),
            timeout_milliseconds(timeout_milliseconds) {}

    void receive(MessagePtr m) override {
        std::lock_guard<std::recursive_mutex> lock(last_message_mutex);
        last_message = m;
        has_new_message_event.set();
    }

    MessagePtr get_latest_message() { 
        std::lock_guard<std::recursive_mutex> lock(last_message_mutex); 
        return last_message; 
    }

    int get_timeout_milliseconds() const { 
        return timeout_milliseconds; 
    }

protected:

    void child_thread_fn() override;
    virtual MessagePtr produce(MessagePtr m) { return m; }

    int timeout_milliseconds;

    // Requires gcc 12.1... do we force upgrade to 12.1?
    // atomic<MessagePtr> last_message;
    mutable std::recursive_mutex last_message_mutex;
    MessagePtr last_message = nullptr;
    util::Event has_new_message_event;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_PRODUCER__H
