#include "flex_utils.h"

namespace roboflex {
namespace serialization {

void serialize_uuid(sole::uuid id, const std::string& key, flexbuffers::Builder& fbb)
{
    char uuidchars[16];
    memcpy(uuidchars, &(id.ab), 8);
    memcpy(uuidchars+8, &(id.cd), 8);
    fbb.Key(key);
    fbb.Blob(uuidchars, 16);
}

const sole::uuid deserialize_uuid(flexbuffers::Blob& blob)
{
    const uint8_t * g = blob.data();
    uint64_t ab = *(uint64_t*)(g);
    uint64_t cd = *(uint64_t*)(g+8);
    return sole::rebuild(ab, cd);
}

} // namespace serialization
} // namespace roboflex
