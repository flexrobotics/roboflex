#ifndef ROBOFLEX_TRANSPORT_TQ__H
#define ROBOFLEX_TRANSPORT_TQ__H

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "core/core.h"

namespace roboflex {
namespace transporttq {

using std::string, std::queue, std::condition_variable, std::mutex;

/**
 * A simple thread-safe queue that can be used
 * as a single pub-sub bridge. Can be used
 * only for thread-thread messaging.
 */
class TQPubSub: public core::RunnableNode {
public:
    TQPubSub(
        const string& name = "TQPubSub",
        unsigned int max_queued_msgs = 1000,
        unsigned int timeout_milliseconds = 10);

    void receive(core::MessagePtr m) override;

protected:

    core::MessagePtr pull_raw();
    void child_thread_fn() override;

    unsigned int max_queued_msgs;
    unsigned int timeout_milliseconds;
    queue<core::MessagePtr> q;
    mutable mutex mutex_;
    condition_variable cv;
};

} // namespace transporttq
} // namespace roboflex

#endif // ROBOFLEX_TRANSPORT_TQ__H

