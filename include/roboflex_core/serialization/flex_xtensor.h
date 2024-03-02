#ifndef ROBOFLEX_CORE_FLEX_TENSOR__H
#define ROBOFLEX_CORE_FLEX_TENSOR__H

#include <iostream>
#include <list>
#include <vector>
#include <string_view>
#include <flatbuffers/flexbuffers.h>
#include <xtensor/xtensor.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xarray.hpp>
#include "flex_tensor_format.h"

namespace roboflex {
namespace serialization {

/**
 * Serialize an xtensor into a flexbuffer Builder as a map with the given name
 * at the root of the tree, with keys "shape", "dtype", and "data":
 * 
 * example:
 * {
 *    "shape": [3, 480, 640],
 *    "dtype": 5,
 *    "data": <blob>,
 * }
 */
template <typename T, size_t NDimensions>
void serialize_flex_tensor(flexbuffers::Builder& fbb, const xt::xtensor<T, NDimensions>& tensor, const std::string& name="")
{
    // get a vector of the shape
    auto shape_vector = std::vector<uint64_t>(tensor.shape().begin(), tensor.shape().end());

    // get the raw bytes
    const T* actual_data = tensor.data();
    const uint8_t* byte_data = (const uint8_t*)actual_data;
    size_t num_bytes = tensor.size() * sizeof(T);

    // write the map key
    if (!name.empty()) {
        fbb.Key(name);
    }

    // write the shape, data, and dtype into the map
    fbb.Map([&]() {
        fbb.Key(ShapeKey);
        fbb.Vector(shape_vector);
        fbb.Key(DataKey);
        fbb.Blob(byte_data, num_bytes);
        fbb.Int(DTypeKey, tensor_dtype_indexer<T>::index);
    });
}

template <typename T>
void serialize_flex_array(flexbuffers::Builder& fbb, const xt::xarray<T>& tensor, const std::string& name="")
{
    // get a vector of the shape
    auto shape_vector = std::vector<uint64_t>(tensor.shape().begin(), tensor.shape().end());

    // get the raw bytes
    const T* actual_data = tensor.data();
    const uint8_t* byte_data = (const uint8_t*)actual_data;
    size_t num_bytes = tensor.size() * sizeof(T);

    // write the map key
    if (!name.empty()) {
        fbb.Key(name);
    }

    // write the shape, data, and dtype into the map
    fbb.Map([&]() {
        fbb.Key(ShapeKey);
        fbb.Vector(shape_vector);
        fbb.Key(DataKey);
        fbb.Blob(byte_data, num_bytes);
        fbb.Int(DTypeKey, tensor_dtype_indexer<T>::index);
    });
}

/**
 * Serialize an xtensor into a flexbuffer Builder as a map with the given name
 * at the root of the tree (which must be a map), with keys "shape", "data", and "dtype". 
 * This version takes not an xtensor, but a shape. It writes a special Blob into the map 
 * under "data" that is basically uninitialized data of the correct size. Later, the 
 * tensor can be written directly into that memory.
 * 
 * example:
 * {
 *    "shape": [3, 480, 640],
 *    "dtype": 5,
 *    "data": <blob>,
 * }
 */
template <typename T, size_t NDimensions>
void serialize_flex_tensor(flexbuffers::Builder& fbb, const std::array<size_t, NDimensions>& tensor_shape, const std::string& name="")
{
    // get a vector of the shape
    auto shape_vector = std::vector<uint64_t>(tensor_shape.begin(), tensor_shape.end());

    // compute the total size
    size_t total_size = 1;
    for (auto v: shape_vector) {
        total_size = total_size * v;
    }
    size_t num_bytes = total_size * sizeof(T);

    // write the map key
    if (!name.empty()) {
        fbb.Key(name);
    }

    // Write the shape, nullptr, and dtype into the map.
    // Writing nullptr for the data will cause the Blob
    // to simply resize itself by num_bytes, and then
    // we can efficiently write an xtensor expression later
    // directly to the memory.
    fbb.Map([&]() {
        fbb.Key(ShapeKey);
        fbb.Vector(shape_vector);
        fbb.Key(DataKey);
        fbb.Blob(nullptr, num_bytes);
        fbb.Int(DTypeKey, tensor_dtype_indexer<T>::index);
    });
}

template <typename T>
void serialize_flex_array(flexbuffers::Builder& fbb, const std::vector<size_t>& tensor_shape, const std::string& name="")
{
    // get a vector of the shape
    auto shape_vector = std::vector<uint64_t>(tensor_shape.begin(), tensor_shape.end());

    // compute the total size
    size_t total_size = 1;
    for (auto v: shape_vector) {
        total_size = total_size * v;
    }
    size_t num_bytes = total_size * sizeof(T);

    // write the map key
    if (!name.empty()) {
        fbb.Key(name);
    }

    // Write the shape, nullptr, and dtype into the map.
    // Writing nullptr for the data will cause the Blob
    // to simply resize itself by num_bytes, and then
    // we can efficiently write an xtensor expression later
    // directly to the memory.
    fbb.Map([&]() {
        fbb.Key(ShapeKey);
        fbb.Vector(shape_vector);
        fbb.Key(DataKey);
        fbb.Blob(nullptr, num_bytes);
        fbb.Int(DTypeKey, tensor_dtype_indexer<T>::index);
    });
}


// The actual type that the eventual call to xt::adapt returns is this.
// (the 'auto' keyword works for the return type as well, but it's
// good to know).
template <typename T>
using flextensor_adaptor = xt::xarray_adaptor<
    xt::xbuffer_adaptor<
        T *&,
        xt::no_ownership,
        std::allocator<T>
    >,
    xt::layout_type::row_major,
    std::vector<unsigned long, std::allocator<unsigned long>>,
    xt::xtensor_expression_tag
>;


/**
 * Deserialize a flexbuffer Reference into an xt::xarray.
 */
template <typename T>
flextensor_adaptor<T> deserialize_flex_array(flexbuffers::Reference r, bool print_memory_address=false)
{
    if (r.IsNull()) {
        throw std::runtime_error("flex_tensor::deserialize_flex_array was handed a null reference");
    }
    if (!is_tensor(r)) {
        throw std::runtime_error("flex_tensor::deserialize_flex_array was not handed a tensor");
    }

    flexbuffers::Map m = r.AsMap();

    // read shape
    std::vector<size_t> vshape = tensor_shape(r);

    // dimension check
    // NONE!!!

    // read datatype code and check against what's being asked for
    auto dtype = tensor_type_code(r);
    if (dtype != tensor_dtype_indexer<T>::index) {
        throw std::runtime_error(
            "flex_tensor::deserialize_flex_array attempted to deserialize tensor of type " +
            std::string(tensor_dtype_namer<T>::name) +
            " (type code: " + std::to_string(tensor_dtype_indexer<T>::index) + ")"
            ", but found a tensor of type " + type_name_from_code(dtype) +
            " (type code: " + std::to_string(dtype) + ").");
    }

    // read actual data
    auto blob = m[DataKey].AsBlob();
    const uint8_t * data_bytes = blob.data();
    const T * data_typed_const = (const T*)data_bytes;

    // yeah yeah yeah
    T* data_typed = const_cast<T*>(data_typed_const);

    if (print_memory_address) {
        std::cout << "deserialize_flex_array found tensor at " << static_cast<const void *>(data_typed) << std::endl;
    }

    // return a NO-OWNERSHIP adapter into the raw data
    // This means the adapter will not destruct memory.
    // This achieves 0-copy!
    return xt::adapt(
        data_typed,
        blob.size() / sizeof(T),
        xt::no_ownership(),
        vshape);
}

/**
 * Deserialize a flexbuffer Reference into an xtensor.
 */
template <typename T, size_t NDimensions>
flextensor_adaptor<T> deserialize_flex_tensor(flexbuffers::Reference r, bool print_memory_address=false)
{
    if (r.IsNull()) {
        throw std::runtime_error("flex_tensor::deserialize_flex_tensor was handed a null reference");
    }
    if (!is_tensor(r)) {
        throw std::runtime_error("flex_tensor::deserialize_flex_tensor was not handed a tensor");
    }

    flexbuffers::Map m = r.AsMap();

    // read shape
    std::vector<size_t> vshape = tensor_shape(r);

    // dimension check
    if (vshape.size() != NDimensions) {
        throw std::runtime_error(
            "flex_tensor::deserialize_flex_tensor attempted to deserialize a tensor of rank " + std::to_string(NDimensions) +
            ", but found a tensor of rank " + std::to_string(vshape.size()) + " (read shape as " + _vec_to_string(vshape) + ").");
    }

    // read datatype code and check against what's being asked for
    auto dtype = tensor_type_code(r);
    if (dtype != tensor_dtype_indexer<T>::index) {
        throw std::runtime_error(
            "flex_tensor::deserialize_flex_tensor attempted to deserialize tensor of type " +
            std::string(tensor_dtype_namer<T>::name) +
            " (type code: " + std::to_string(tensor_dtype_indexer<T>::index) + ")"
            ", but found a tensor of type " + type_name_from_code(dtype) +
            " (type code: " + std::to_string(dtype) + ").");
    }

    // read actual data
    auto blob = m[DataKey].AsBlob();
    const uint8_t * data_bytes = blob.data();
    const T * data_typed_const = (const T*)data_bytes;

    // yeah yeah yeah
    T* data_typed = const_cast<T*>(data_typed_const);

    if (print_memory_address) {
        std::cout << "deserialize_flex_tensor found tensor at " << static_cast<const void *>(data_typed) << std::endl;
    }

    // return a NO-OWNERSHIP adapter into the raw data
    // This means the adapter will not destruct memory.
    // This achieves 0-copy!
    return xt::adapt(
        data_typed,
        blob.size() / sizeof(T),
        xt::no_ownership(),
        vshape);
}

/**
 * Deserialize a flexbuffer Reference into an xtensor, but don't throw
 * if the dimensions are wrong.
 */
template <typename T>
flextensor_adaptor<T> deserialize_flex_tensor_no_dim_check(flexbuffers::Reference r)
{
    if (r.IsNull()) {
        throw std::runtime_error("flex_tensor::deserialize_flex_tensor_no_dim_check was handed a null reference");
    }
    if (!is_tensor(r)) {
        throw std::runtime_error("flex_tensor::deserialize_flex_tensor_no_dim_check was not handed a tensor");
    }

    flexbuffers::Map m = r.AsMap();
    
    // read shape
    std::vector<size_t> vshape = tensor_shape(r);

    // read datatype code and check against what's being asked for
    auto dtype = tensor_type_code(r);
    if (dtype != tensor_dtype_indexer<T>::index) {
        throw std::runtime_error(
            "flex_tensor::deserialize_flex_tensor attempted to deserialize tensor of type " +
            std::string(tensor_dtype_namer<T>::name) +
            " (type code: " + std::to_string(tensor_dtype_indexer<T>::index) + ")"
            ", but found a tensor of type " + type_name_from_code(dtype) +
            " (type code: " + std::to_string(dtype) + ").");
    }

    // read actual data
    auto blob = m[DataKey].AsBlob();
    const uint8_t * data_bytes = blob.data();
    const T * data_typed_const = (const T*)data_bytes;

    // yeah yeah yeah
    T* data_typed = const_cast<T*>(data_typed_const);

    // return a NO-OWNERSHIP adapter into the raw data
    // This means the adapter will not destruct memory.
    return xt::adapt(
        data_typed,
        blob.size() / sizeof(T),
        xt::no_ownership(),
        vshape);
}

} // namespace serialization
} // namespace roboflex

#endif // ROBOFLEX_CORE_FLEX_TENSOR__H
