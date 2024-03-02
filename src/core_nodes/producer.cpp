#include "roboflex_core/core_nodes/producer.h"

namespace roboflex {
namespace nodes {

void Producer::child_thread_fn()
{
    while (!this->stop_requested()) {
        bool has_new_message = has_new_message_event.wait_once(timeout_milliseconds);
        if (has_new_message) {
            has_new_message_event.clear();
            MessagePtr m = get_latest_message();
            auto emitted_message = this->produce(m);
            this->signal(emitted_message);
        }
    }
}

} // namespace nodes
} // namespace roboflex
