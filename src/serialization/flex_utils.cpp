#include "roboflex_core/serialization/flex_utils.h"

/** 
 * What's going on here? Basically, this is the only
 * static variable that doesn't come from the flexbuffers
 * library. We gotta initialize it, or compile in src/utils.cpp.
 * So this is a big hack.
 */
namespace flatbuffers {
    #ifdef _MSC_VER
    ClassicLocale::ClassicLocale()
        : locale_(_create_locale(LC_ALL, "C")) {}
    ClassicLocale::~ClassicLocale() { _free_locale(locale_); }
    #else
    ClassicLocale::ClassicLocale()
        : locale_(newlocale(LC_ALL, "C", nullptr)) {}
    ClassicLocale::~ClassicLocale() { freelocale(locale_); }
    #endif
    ClassicLocale ClassicLocale::instance_;
}

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
