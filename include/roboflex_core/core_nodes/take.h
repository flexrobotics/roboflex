#ifndef ROBOFLEX_TAKE__H
#define ROBOFLEX_TAKE__H

#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

using std::vector, std::shared_ptr;

vector<MessagePtr> take(size_t n, Node& from, int timeout_milliseconds=0);
vector<MessagePtr> take(size_t n, shared_ptr<Node> from, int timeout_milliseconds=0);

MessagePtr take1(Node& from, int timeout_milliseconds=0);
MessagePtr take1(shared_ptr<Node> from, int timeout_milliseconds=0);

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_TAKE__H
