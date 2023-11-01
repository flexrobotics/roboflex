#ifndef ROBOFLEX_SERIALIZATION_CORE_FLEX_UTILS__H
#define ROBOFLEX_SERIALIZATION_CORE_FLEX_UTILS__H

#include <flatbuffers/flexbuffers.h>
#include "roboflex_core/util/uuid.h"

namespace roboflex {
namespace serialization {

/**
 * Serialize a sole::uuid to a flexbuffer builder.
 */
void serialize_uuid(sole::uuid id, const std::string& key, flexbuffers::Builder& fbb);

/**
 * Deserialize a sole::uuid from a flexbuffer blob.
 */
const sole::uuid deserialize_uuid(flexbuffers::Blob& blob);

/**
 * Constructs a string from a vector representing a shape.
 */
std::string shape_to_string(const std::vector<size_t>& shape);

} // namespace serialization
} // namespace roboflex

#endif // ROBOFLEX_SERIALIZATION_CORE_FLEX_UTILS__H
