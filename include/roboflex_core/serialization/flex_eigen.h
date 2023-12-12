#ifndef ROBOFLEX_CORE_FLEX_EIGEN__H
#define ROBOFLEX_CORE_FLEX_EIGEN__H

#include <iostream>
#include <list>
#include <vector>
#include <string_view>
#include <Eigen/Dense>
#include <flatbuffers/flexbuffers.h>
#include "flex_tensor_format.h"

namespace roboflex {
namespace serialization {

template <typename T, int NRows, int NCols, int Options=Eigen::ColMajor>
void serialize_eigen_matrix(flexbuffers::Builder& fbb, const Eigen::Matrix<T, NRows, NCols, Options>& matrix, const std::string& name="")
{
    // get a vector of the shape
    std::vector<uint64_t> shape_vector = { (uint64_t)matrix.rows(), (uint64_t)matrix.cols() };

    // get the raw bytes
    const T* actual_data = matrix.data();
    const uint8_t* byte_data = (const uint8_t*)actual_data;
    size_t num_bytes = matrix.size() * sizeof(T);

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


template <typename T, int NRows, int NCols, int Options=Eigen::ColMajor>
Eigen::Map<const Eigen::Matrix<T, NRows, NCols, Options>> deserialize_eigen_matrix(flexbuffers::Reference r)
{
    if (r.IsNull()) {
        throw std::runtime_error("flex_tensor::deserialize_eigen_matrix was handed a null reference");
    }
    if (!is_tensor(r)) {
        throw std::runtime_error("flex_tensor::deserialize_eigen_matrix was not handed a tensor");
    }

    flexbuffers::Map m = r.AsMap();

    // read shape
    std::vector<size_t> vshape = tensor_shape(r);

    // dimension check: for now we only support rank 2 (matrixes)
    if (vshape.size() != 2) {
        throw std::runtime_error(
            "flex_eigen::deserialize_eigen_matrix attempted to deserialize a matrix of rank " + std::to_string(2) +
            ", but found a matrix of rank " + std::to_string(vshape.size()) + " (read shape as " + _vec_to_string(vshape) + ").");
    }

    // another dimension check: check the actual dimensions
    if (NRows != Eigen::Dynamic) {
        if (vshape[0] != NRows) {
            throw std::runtime_error(
                "flex_eigen::deserialize_eigen_matrix expected " + std::to_string(NRows) +
                " rows, but found " + std::to_string(vshape[0]) + ".");
        }
    }
    if (NCols != Eigen::Dynamic) {
        if (vshape[1] != NCols) {
            throw std::runtime_error(
                "flex_eigen::deserialize_eigen_matrix expected " + std::to_string(NCols) +
                " cols, but found " + std::to_string(vshape[1]) + ".");
        }
    }

    // read datatype code and check against what's being asked for
    auto dtype = tensor_type_code(r);
    if (dtype != tensor_dtype_indexer<T>::index) {
        throw std::runtime_error(
            "flex_eigen::deserialize_eigen_matrix attempted to deserialize matrix of type " +
            std::string(tensor_dtype_namer<T>::name) +
            " (type code: " + std::to_string(tensor_dtype_indexer<T>::index) + ")"
            ", but found a matrix of type " + type_name_from_code(dtype) +
            " (type code: " + std::to_string(dtype) + ").");
    }

    // read actual data
    auto blob = m[DataKey].AsBlob();
    const uint8_t * data_bytes = blob.data();
    const T * data_typed = (const T*)data_bytes;

    // Return a Map object. If all is well, then this
    // should perform no copies - it should basically
    // be a 'view' over the blob's data.
    auto retval = Eigen::Map<const Eigen::Matrix<T, NRows, NCols, Options>>(
        data_typed,
        vshape[0],
        vshape[1]);

    return retval;
}

} // namespace serialization
} // namespace roboflex

#endif // ROBOFLEX_CORE_FLEX_EIGEN__H
