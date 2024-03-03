#ifndef ROBOFLEX_TENSOR_BUFFER__H
#define ROBOFLEX_TENSOR_BUFFER__H

#include "roboflex_core/node.h"
#include "roboflex_core/serialization/flex_xtensor.h"
#include <xtensor/xview.hpp>

namespace roboflex {
using namespace core;
namespace nodes {

using xt::placeholders::_;

template <typename TensorType>
void shift_left_add(
    TensorType& buffer,
    const serialization::flextensor_adaptor<typename TensorType::value_type>& new_tensor
) {
    // Get the size of the last dimension of t
    int last_t_size = new_tensor.shape().back();

    // Shift the buffer to the left
    auto start_slice = xt::strided_view(buffer, {xt::ellipsis(), xt::range(_, -last_t_size)});
    auto assign_slice = xt::strided_view(buffer, {xt::ellipsis(), xt::range(last_t_size, _)});
    start_slice.assign(assign_slice);

    // Copy t into the end of the buffer
    auto end_slice = xt::strided_view(buffer, {xt::ellipsis(), xt::range(-last_t_size, _)});
    
    // this doesn't work for some reason
    // end_slice = new_tensor;
    end_slice.assign(new_tensor);
}

template <typename TensorType>
void chop(TensorType& buffer, size_t len, typename TensorType::value_type value) {
    auto slice = xt::strided_view(buffer, {xt::ellipsis(), xt::range(_, len)});
    slice.fill(value);
}

template <typename T>
struct XArrayRightBuf {
    using BufferTensorType = xt::xarray<T>;
    XArrayRightBuf(const std::vector<size_t>& shape): buffer(shape) {}
    BufferTensorType& add(const serialization::flextensor_adaptor<T>& t) {
        shift_left_add(buffer, t);
        return buffer;
    }
    void chop(size_t len, T value) { roboflex::nodes::chop(buffer, len, value); }
    BufferTensorType buffer;
};

// A specialization is needed for half_float. Lame.
template <>
struct XArrayRightBuf<xtl::half_float> {
    using BufferTensorType = xt::xarray<xtl::half_float>;
    XArrayRightBuf(const std::vector<size_t>& shape): buffer(shape, xt::layout_type::row_major) {}
    BufferTensorType& add(const serialization::flextensor_adaptor<xtl::half_float>& t) {
        shift_left_add(buffer, t);
        return buffer;
    }
    void chop(size_t len, xtl::half_float value) { roboflex::nodes::chop(buffer, len, value); }
    BufferTensorType buffer;
};

template <typename T, size_t NDimensions>
struct XTensorRightBuf {
    using BufferTensorType = xt::xtensor<T, NDimensions>;
    XTensorRightBuf(const std::array<size_t, NDimensions>& shape): buffer(shape) {}
    BufferTensorType& add(const serialization::flextensor_adaptor<T>& t) {
        shift_left_add(buffer, t);
        return buffer;
    }
    void chop(size_t len, T value) { roboflex::nodes::chop(buffer, len, value); }
    BufferTensorType buffer;
};

template <typename T, size_t ...Shape>
struct XTensorFixedRightBuf {
    using BufferTensorType = xt::xtensor_fixed<T, xt::xshape<Shape...>>;
    XTensorFixedRightBuf(): buffer() {}
    BufferTensorType& add(const serialization::flextensor_adaptor<T>& t) {
        shift_left_add(buffer, t);
        return buffer;
    }
    void chop(size_t len, T value) { roboflex::nodes::chop(buffer, len, value); }
    BufferTensorType buffer;
};

/**
 * A Node that buffers tensors in a left-shifting buffer.
 * New information is always at the right end.
 */
template <typename T>
class TensorRightBuffer: public Node {
public:
    TensorRightBuffer(
        const std::vector<size_t>& shape,
        const std::string& tensor_key_in = "t",
        const std::string& tensor_key_out = "buffer",
        const std::string& count_key_out = "count",
        const std::string& name = "TensorBuffer"):
            Node(name),
            tensor_key_in(tensor_key_in),
            tensor_key_out(tensor_key_out),
            count_key_out(count_key_out),
            buf(shape) {}

    void receive(MessagePtr m) override;
    std::string to_string() const override;
    void chop(size_t len, T value);

    // hmm - sort of difficult
    //XArrayRightBuf<T>& get_buffer() { return buf; }

protected:

    std::string tensor_key_in;
    std::string tensor_key_out;
    std::string count_key_out;
    mutable std::recursive_mutex buffer_mutex;
    XArrayRightBuf<T> buf;
    uint64_t count = 0;
};

template <typename T>
void TensorRightBuffer<T>::receive(MessagePtr m) 
{
    // get the map at the key holding the tensor
    auto tensor_map = m->root_val(tensor_key_in);

    // deserialize into an xtensor adapter... no copy yet!
    serialization::flextensor_adaptor<T> tensor_adapter = serialization::deserialize_flex_array<T>(tensor_map);

    // lock the buffer
    std::lock_guard<std::recursive_mutex> lock(buffer_mutex);

    // increment my total count of the last dimension
    count += tensor_adapter.shape().back();

    // add the tensor to the buffer
    auto buffer_tensor = buf.add(tensor_adapter);

    // create a new message with the buffer tensor
    auto msg = std::make_shared<TensorBufferMessage<T>>(buffer_tensor, count, tensor_key_out, count_key_out);

    // signal it
    this->signal(msg);
}


template <typename T>
std::string TensorRightBuffer<T>::to_string() const
{
    std::stringstream sst;
    sst << "<TensorRightBuffer (" << count << ") bufshape=" << xt::adapt(buf.buffer.shape()) << " " << Node::to_string() << ">";
    return sst.str();
}

template <typename T>
void TensorRightBuffer<T>::chop(size_t len, T value) 
{
    std::lock_guard<std::recursive_mutex> lock(buffer_mutex); 
    buf.chop(len, value);
}

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_TENSOR_BUFFER__H
