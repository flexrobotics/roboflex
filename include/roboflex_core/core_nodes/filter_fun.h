#ifndef ROBOFLEX_FILTER_FUN__H
#define ROBOFLEX_FILTER_FUN__H

#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * A Node that calls a std::function predicate to filter messages.
 */
class FilterFun: public Node {
public:
    using FilterFunction = std::function<bool(MessagePtr)>;

    FilterFun(FilterFunction filterfun, const std::string& name = "FilterFun"):
        Node(name), _filterfun(filterfun) {}

    void receive(MessagePtr m) override {
        if (_filterfun(m)) {
            signal(m);
        }
    }

protected:
    FilterFunction _filterfun;
};

/**
 * A Node that filters by message name.
 */
class FilterName: public Node {
public:
    FilterName(const std::string& message_name, const std::string& name = "FilterName"):
        Node(name), _message_name(message_name) {}

    void receive(MessagePtr m) override {
        if (m->message_name() == _message_name) {
            signal(m);
        }
    }

protected:
    std::string _message_name;
};

/**
 * A Node that filters by message name, with a passthrough boolean.
 */
class FilterNamePassthrough: public Node {
public:
    FilterNamePassthrough(const std::string& message_name, bool initial_passthrough, const std::string& name = "FilterNamePassthrough"):
        Node(name), _message_name(message_name), passthrough(initial_passthrough) {}

    void set_passthrough(bool p) { passthrough = p; }
    bool get_passthrough() const { return passthrough; }

    void receive(MessagePtr m) override {
        if (passthrough || (m->message_name() == _message_name)) {
            signal(m);
        }
    }

protected:
    std::string _message_name;
    std::atomic<bool> passthrough;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_FILTER_FUN__H
