#include "roboflex_core/serialization/flex_tensor_format.h"

namespace roboflex {
namespace serialization {

std::string type_name_from_code(int type_code) {
    switch (type_code) {
        case 0: return "int8_t";
        case 1: return "int16_t";
        case 2: return "int32_t";
        case 3: return "int64_t";
        case 4: return "uint8_t";
        case 5: return "uint16_t";
        case 6: return "uint32_t";
        case 7: return "uint64_t";
        //case 8: return "intp";
        //case 9: return "uintp";
        case 10: return "float";
        case 11: return "double";
        //case 12: return "complex64";
        //case 13: return "complex128";
        case 14: return "float16";
        default: return "wat";
    }
}

static std::map<std::string, int> TypeNamesToCodes = {
    { "int8_t", 0 },
    { "int16_t", 1 },
    { "int32_t", 2 },
    { "int64_t", 3 },
    { "uint8_t", 4 },
    { "uint16_t", 5 },
    { "uint32_t", 6 },
    { "uint64_t", 7 },
    // { "intp", 8 },
    // { "uintp", 9 },
    { "float", 10 },
    { "double", 11 },
    // { "complex64", 12 },
    // { "complex128", 13 },
    { "float16", 14 },
    { "wat", -1 }
};

int type_code_from_name(const std::string& type_name) 
{
    return TypeNamesToCodes[type_name];
}

bool is_tensor(flexbuffers::Reference r) 
{
    return r.IsMap() && r.AsMap()[DataKey].IsBlob() && r.AsMap()[DTypeKey].IsInt() && (r.AsMap()[ShapeKey].IsVector() || r.AsMap()[ShapeKey].IsTypedVector());
}

int tensor_type_code(flexbuffers::Reference r) 
{
    return (r.IsMap() && r.AsMap()[DTypeKey].IsInt()) ? r.AsMap()[DTypeKey].AsInt8() : -1;
}

std::string tensor_type_name(flexbuffers::Reference r)
{
    return type_name_from_code(tensor_type_code(r));
}

int tensor_rank(flexbuffers::Reference r) 
{
    if (!r.IsMap()) {
        return -1;
    }
    auto m = r.AsMap();
    auto s = m[ShapeKey];
    return s.IsVector() ? s.AsVector().size() : s.IsTypedVector() ? s.AsTypedVector().size() : -1;
}

std::vector<size_t> tensor_shape(flexbuffers::Reference r) 
{
    if (!r.IsMap() || !(r.AsMap()[ShapeKey].IsVector() || r.AsMap()[ShapeKey].IsTypedVector())) {
        return {};
    }
    auto m = r.AsMap();
    auto shape = m[ShapeKey].AsTypedVector();
    std::vector<size_t> vshape;
    for (size_t i=0; i<shape.size(); i++) {
        uint64_t shape_element = shape[i].AsUInt64();
        vshape.push_back(shape_element);
    }
    return vshape;
}

} // namespace serialization
} // namespace roboflex
