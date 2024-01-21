#ifndef ROBOFLEX_NULL__H
#define ROBOFLEX_NULL__H

#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

/**
 * Syntactic sugar for Node("null").
 */
class Null: public Node {
public:
    Null(const std::string& name = "null"): Node(name) {}
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_NULL__H
