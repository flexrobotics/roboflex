#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include "roboflex_core/core_nodes/callback_fun.h"
#include "roboflex_core/core_nodes/take.h"
#include "roboflex_core/util/event.h"

namespace roboflex {
namespace nodes {

std::vector<MessagePtr> take(size_t n, Node& from, int timeout_milliseconds)
{
    std::vector<MessagePtr> messages;

    if (n == 0) {
        return messages;
    }

    util::Event e;

    auto callback = CallbackFun([&messages, &e, n](MessagePtr m) {
        if (messages.size() < n) {
            messages.push_back(m);
            if (messages.size() == n) {
                e.set();
            }
        }
    });

    from.connect(callback);
    e.wait(timeout_milliseconds);
    from.disconnect(callback);

    return messages;
}

std::vector<MessagePtr> take(size_t n, std::shared_ptr<Node> from, int timeout_milliseconds)
{
    return take(n, *from, timeout_milliseconds);
}

MessagePtr take1(Node& from, int timeout_milliseconds)
{
    std::vector<MessagePtr> res = take(1, from, timeout_milliseconds);
    if (res.size() > 0) {
        return res[0];
    }
    return nullptr;
}

MessagePtr take1(std::shared_ptr<Node> from, int timeout_milliseconds)
{
    return take1(*from, timeout_milliseconds);
}

} // namespace nodes
} // namespace roboflex
