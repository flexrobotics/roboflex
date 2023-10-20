#ifndef ROBOFLEX_CALLBACK_FUN__H
#define ROBOFLEX_CALLBACK_FUN__H

#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * A Node that calls a std::function, but passes its message through unchanged.
 * Works with lambdas!
 */
class CallbackFun: public Node {
public:
    using CallbackFunction = std::function<void(MessagePtr)>;

    CallbackFun(CallbackFunction callbackfun, const std::string& name = "CallbackFun"):
        Node(name), _callbackfun(callbackfun) {}

    void receive(MessagePtr m) override {
        _callbackfun(m);
        signal(m);
    }

protected:
    CallbackFunction _callbackfun;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_CALLBACK_FUN__H
