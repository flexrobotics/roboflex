#ifndef ROBOFLEX_MAP_FUN__H
#define ROBOFLEX_MAP_FUN__H

#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * Maps an std::function - a function that takes a MessagePtr and returns a MessagePtr.
 * Signals whatever the mapped function returns.
 */
class MapFun: public Node {
public:
    using MapFunction = std::function<MessagePtr(MessagePtr)>;

    MapFun(MapFunction mapfun, const std::string& name = "MapFun"):
        Node(name), _mapfun(mapfun) {}

    void receive(MessagePtr m) override {
        auto r = _mapfun(m);
        if (r != nullptr) {
            signal(r);
        }
    }

protected:
    MapFunction _mapfun;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_MAP_FUN__H
