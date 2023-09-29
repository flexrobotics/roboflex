#include "tq.h"

namespace roboflex {
namespace transporttq {

using std::lock_guard, std::unique_lock;

TQPubSub::TQPubSub(const string& name,
    unsigned int max_queued_msgs,
    unsigned int timeout_milliseconds):
        core::RunnableNode(name),
        max_queued_msgs(max_queued_msgs),
        timeout_milliseconds(timeout_milliseconds)
{

}

void TQPubSub::receive(core::MessagePtr m) {
    lock_guard<mutex> lock(mutex_);
    q.push(m);
    if (q.size() > max_queued_msgs) {
        q.pop();
    }
    cv.notify_one();
}

core::MessagePtr TQPubSub::pull_raw()
{
    unique_lock<mutex> lock(mutex_);
    if (q.empty()) {
        cv.wait_for(lock, timeout_milliseconds*1ms);
    }
    if (!q.empty()) {
        auto m = q.front();
        q.pop();
        return m;
    }
    return nullptr;
}

void TQPubSub::child_thread_fn() {
    while (!this->stop_signal) {
        auto m = pull_raw();
        if (m != nullptr) {
            signal(m);
        }
    }
}

} // namespace transporttq
} // namespace roboflex
