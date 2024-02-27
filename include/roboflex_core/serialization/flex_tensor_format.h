/**
 * Serialize and deserialize xtensors and eigen matrices to and from flexbuffers.
 * Like so:
 *
 * SERIALIZE:
 *     template <typename T, size_t NDimensions>
 *     void serialize_flex_tensor(
 *         flexbuffers::Builder& fbb,
 *         const xt::xtensor<T, NDimensions>& tensor,
 *         const std::string& name="") {
 *
 * DESERIALIZE:
 *      template <typename T, size_t NDimensions>
 *      flextensor_adaptor<T> deserialize_flex_tensor(flexbuffers::Map&)
 * 
 * (and equivalent for eigen matrices - see flex_xtensor.h and flex_eigen.h).
 *
 *
 * Code to serialize and deserialize xtensors to and from flexbuffers,
 * in the exact same way that flextensors.py does it, as a map with
 * three entries:
 *   
 *    {
 *      "shape": FixedTypedVector
 *      "data": Blob
 *      "dtype": data type index, UInt8
 *    }
 *
 * That data type index is an index representing the tensor numeric type
 * (int32, float, etc), and is an index into this array:
 *
 * supported_numpy_types = [
        np.int8, np.int16, np.int32, np.int64,
        np.uint8, np.uint16, np.uint32, np.uint64,
        np.intp, np.uintp,
        np.float32, np.float64,
        np.complex64, np.complex128,
        np.float16
    ]
 *
 * All the fancy template code you see is designed to turn a numeric
 * type into the same integer index, and get a string.
 *
 * NOTE: not sure what to do about the types 'np.intp' and 'np.uintp',
 * which are 'index types'. Their corresponding c types are intptr_t and uintptr_t,
 * but those conflict with int64_t (if a pointer is 64 bits) and uint64_t...
 * And, those types may be important for transferring index tensors...
 *
 * Less worried, for the moment, about complex types.
 */

#ifndef ROBOFLEX_SERIALIZATION_CORE_FLEX_TENSOR_FORMAT__H
#define ROBOFLEX_SERIALIZATION_CORE_FLEX_TENSOR_FORMAT__H

#include <flatbuffers/flexbuffers.h>
#include <xtl/xhalf_float.hpp>

namespace roboflex {
namespace serialization {

constexpr char DataKey[] = "data";
constexpr char ShapeKey[] = "shape";
constexpr char DTypeKey[] = "dtype";

bool is_tensor(flexbuffers::Reference r);
int tensor_type_code(flexbuffers::Reference r);
std::string tensor_type_name(flexbuffers::Reference r);
int tensor_rank(flexbuffers::Reference r);
std::vector<size_t> tensor_shape(flexbuffers::Reference r);

std::string type_name_from_code(int type_code);
int type_code_from_name(const std::string& type_name);

template <typename T> struct tensor_dtype_indexer { constexpr static int index = -1; };
template <> struct tensor_dtype_indexer<int8_t> { constexpr static int index = 0; };
template <> struct tensor_dtype_indexer<int16_t> { constexpr static int index = 1; };
template <> struct tensor_dtype_indexer<int32_t> { constexpr static int index = 2; };
template <> struct tensor_dtype_indexer<int64_t> { constexpr static int index = 3; };
template <> struct tensor_dtype_indexer<uint8_t> { constexpr static int index = 4; };
template <> struct tensor_dtype_indexer<uint16_t> { constexpr static int index = 5; };
template <> struct tensor_dtype_indexer<uint32_t> { constexpr static int index = 6; };
template <> struct tensor_dtype_indexer<uint64_t> { constexpr static int index = 7; };
//template <> struct tensor_dtype_indexer<intptr_t> { constexpr static int index = 8; };      // intp in numpy
//template <> struct tensor_dtype_indexer<uintptr_t> { constexpr static int index = 9; };     // uintp in numpy
template <> struct tensor_dtype_indexer<float> { constexpr static int index = 10; };
template <> struct tensor_dtype_indexer<double> { constexpr static int index = 11; };
//template <> struct tensor_dtype_indexer<complex64> { constexpr static int index = 12; };
//template <> struct tensor_dtype_indexer<complex128> { constexpr static int index = 13; };
template <> struct tensor_dtype_indexer<xtl::half_float> { constexpr static int index = 14; };

template <typename T> struct tensor_dtype_namer { constexpr static std::string_view name = "wat"; };
template <> struct tensor_dtype_namer<int8_t> { constexpr static std::string_view name = "int8_t"; };
template <> struct tensor_dtype_namer<int16_t> { constexpr static std::string_view name = "int16_t"; };
template <> struct tensor_dtype_namer<int32_t> { constexpr static std::string_view name = "int32_t"; };
template <> struct tensor_dtype_namer<int64_t> { constexpr static std::string_view name = "int64_t"; };
template <> struct tensor_dtype_namer<uint8_t> { constexpr static std::string_view name = "uint8_t"; };
template <> struct tensor_dtype_namer<uint16_t> { constexpr static std::string_view name = "uint16_t"; };
template <> struct tensor_dtype_namer<uint32_t> { constexpr static std::string_view name = "uint32_t"; };
template <> struct tensor_dtype_namer<uint64_t> { constexpr static std::string_view name = "uint64_t"; };
//template <> struct tensor_dtype_namer<intptr_t> { constexpr static std::string_view name = "intp"; };
//template <> struct tensor_dtype_namer<uintptr_t> { constexpr static std::string_view name = "uintp"; };
template <> struct tensor_dtype_namer<float> { constexpr static std::string_view name = "float"; };
template <> struct tensor_dtype_namer<double> { constexpr static std::string_view name = "double"; };
//template <> struct tensor_dtype_namer<complex64> { constexpr static std::string_view name = "complex64"; };
//template <> struct tensor_dtype_namer<complex128> { constexpr static std::string_view name = "complex128"; };
template <> struct tensor_dtype_namer<xtl::half_float> { constexpr static std::string_view name = "float16"; };


// useful for debugging
template <typename T>
std::string _vec_to_string(const std::vector<T>& v)
{
    std::stringstream sst;
    sst << "[";
    bool first = true;
    for (auto value: v) {
        if (!first) {
            sst << ", ";
        }
        first = false;
        sst << std::to_string(value);
    }
    sst << "]";
    return sst.str();
}

} // namespace serialization
} // namespace roboflex

#endif // ROBOFLEX_SERIALIZATION_CORE_FLEX_TENSOR_FORMAT__H
