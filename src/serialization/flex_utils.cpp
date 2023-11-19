#include "roboflex_core/serialization/flex_utils.h"

/** 
 * What's going on here? Basically, this is the only
 * static variable that doesn't come from the flexbuffers
 * library. We gotta initialize it, or compile in src/utils.cpp.
 * So this is a big hack.
 */
namespace flatbuffers {
#if defined(__APPLE__) && defined(__MACH__)
    // apparently we don't need to do it on mac...
#else
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
#endif
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
    uint64_t ab = *(const uint64_t*)(g);
    uint64_t cd = *(const uint64_t*)(g+8);
    return sole::rebuild(ab, cd);
}

std::string shape_to_string(const std::vector<size_t>& shape)
{
    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < shape.size(); i++) {
        ss << shape[i];
        if (i < shape.size()-1) {
            ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}

} // namespace serialization
} // namespace roboflex
