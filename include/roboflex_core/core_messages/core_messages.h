#ifndef ROBOFLEX_CORE_COREMESSAGES__H
#define ROBOFLEX_CORE_COREMESSAGES__H

/**
 * A little library of core messages that are used in many places.
 */

#include <xtensor/xfunction.hpp>
#include <xtensor/xio.hpp>
#include <Eigen/Dense>
#include "flatbuffers/flexbuffers.h"
#include "roboflex_core/message.h"
#include "roboflex_core/message_backing_store.h"
#include "roboflex_core/serialization/flex_xtensor.h"
#include "roboflex_core/serialization/flex_eigen.h"

using namespace std;

namespace roboflex::core {

constexpr char CoreModuleName[] = "core";

/**
 * A message with no data.
 */
class BlankMessage: public Message {
public:
    BlankMessage(Message& other): Message(other) {}
    BlankMessage(const string& message_name):
        Message(CoreModuleName, message_name)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [](){}); // still have to do this...
    }

    void print_on(ostream& os) const override {
        os << "<BlankMessage ";
        Message::print_on(os);
        os << ">";
    }
};

/**
 * A message with a string under the key "s".
 */
class StringMessage: public Message {
public:
    StringMessage(Message& other): Message(other) {}

    StringMessage(const string& message_name, const string& message):
        Message(CoreModuleName, message_name)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            fbb.String("s", message);
        });
    }

    const string message() const {
        return root_map()["s"].AsString().str();
    }

    void print_on(ostream& os) const override {
        os << "<StringMessage s:" << this->message() << " ";
        Message::print_on(os);
        os << ">";
    }
};

/**
 * A message with a float under the key "v".
 */
class FloatMessage: public Message {
public:
    FloatMessage(Message& other): Message(other) {}

    FloatMessage(const string& message_name, const float v):
        Message(CoreModuleName, message_name)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            fbb.Float("v", v);
        });
    }

    float value() const {
        return root_map()["v"].AsFloat();
    }

    void print_on(ostream& os) const override {
        os << "<FloatMessage v:" << this->value() << " ";
        Message::print_on(os);
        os << ">";
    }
};

/**
 * A message with a double under the key "v".
 */
class DoubleMessage: public Message {
public:
    DoubleMessage(Message& other): Message(other) {}

    DoubleMessage(const string& message_name, const double v):
        Message(CoreModuleName, message_name)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            fbb.Float("v", v);
        });
    }

    float value() const {
        return root_map()["v"].AsDouble();
    }

    void print_on(ostream& os) const override {
        os << "<DoubleMessage v:" << this->value() << " ";
        Message::print_on(os);
        os << ">";
    }
};

/**
 * Carries a single tensor under some map key, default of "t".
 * Supports writing: you may overwrite the tensor
 * with a new one of the exact same shape and dtype.
 */
template <typename T, size_t Rank>
class TensorMessage: public Message {
public:

    inline static const string DefaultMessageName = "TensorMessage";
    inline static const string DefaultKey = "t";

    TensorMessage(Message& other, const string& key="t"): Message(other), key(key) {}

    TensorMessage(const xt::xtensor<T, Rank>& matrix, const string& message_name=DefaultMessageName, const string& key=DefaultKey):
        Message(CoreModuleName, message_name, nullptr), key(key)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            serialization::serialize_flex_tensor<T, Rank>(fbb, matrix, key);
        });
    }

    TensorMessage(const std::array<size_t, Rank>& shape, const string& message_name=DefaultMessageName, const string& key=DefaultKey):
        Message(message_name, nullptr), key(key)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            serialization::serialize_flex_tensor<T, Rank>(fbb, shape, key);
        });
    }

    static shared_ptr<TensorMessage> Ptr(const xt::xtensor<T, Rank>& tensor, const string& message_name=DefaultMessageName, const string& key=DefaultKey) {
        return std::make_shared<TensorMessage<T, Rank>>(tensor, message_name, key);
    }

    template <typename ... whatever>
    static shared_ptr<TensorMessage> Ptr(const xt::xfunction<whatever...>& f, const string& message_name=DefaultMessageName, const string& key=DefaultKey) {
        auto t = std::make_shared<TensorMessage<T, Rank>>(f.shape(), message_name, key);
        t->set_value(f);
        return t;
    }

    void print_on(ostream& os) const override {
        auto root = root_map()[this->key].AsMap();
        auto dtype = root["dtype"].AsInt8();
        os << "<TensorMessage" << " key:" << this->key 
           << " shape:" << xt::adapt(this->value().shape()) 
           << " dtype:" << serialization::type_name_from_code(dtype) 
           << " ";
        Message::print_on(os);
        os << ">";
    }

    const serialization::flextensor_adaptor<T> value() const {
        auto root = root_map()[this->key];
        return serialization::deserialize_flex_tensor<T, Rank>(root);
    }

    // If you have a realized tensor, use this to overwrite the data.
    void set_value(const xt::xtensor<T, Rank>& x) {

        // my root must be a map that obeys our tensor format
        auto root = root_map()[this->key].AsMap();

        // get, ultimately, a pointer to the data that backs the tensor
        auto tensor_data_portion = root["data"].AsBlob();
        const uint8_t* tensor_const_data = tensor_data_portion.data();
        uint8_t* tensor_data = const_cast<uint8_t*>(tensor_const_data);

        // get the raw bytes of the new data
        const T* new_data = x.data();
        const uint8_t* byte_data = (const uint8_t*)new_data;
        size_t num_bytes = x.size() * sizeof(T);

        // perform a copy
        memcpy((void*)tensor_data, (const void*)byte_data, num_bytes);
    }

    // If you have an xtensor-returning lazy function, use this to overwrite the data.
    template<typename ... whatever>
    void set_value(const xt::xfunction<whatever...>& f) {

        // my root must be a map that obeys our tensor format
        auto root = root_map()[this->key].AsMap();

        // get, ultimately, a pointer to the data that backs the tensor
        auto tensor_data_portion = root["data"].AsBlob();
        const uint8_t* tensor_const_data = tensor_data_portion.data();
        uint8_t* tensor_data = const_cast<uint8_t*>(tensor_const_data);
        T * data_typed = (T*)tensor_data;

        // get an adapter over the data, and assign it
        auto z = xt::adapt(
            data_typed,
            tensor_data_portion.size() / sizeof(T),
            xt::no_ownership(),
            f.shape());

        // Evaluate the function directly into the adapter,
        // and hence the underlying memory. If everything went right,
        // there will have been 0 intermediate copies created.
        z = f;
    }

protected:

    string key;
};

/**
 * Carries a single eigen matrix under some map key.
 */
template <typename T, int NRows, int NCols, int Options=Eigen::ColMajor>
class EigenMessage: public Message {
public:

    inline static const string DefaultMessageName = "EigenMessage";
    inline static const string DefaultKey = "t";

    EigenMessage(Message& other, const string& key="t"): Message(other), key(key) {}

    EigenMessage(const Eigen::Matrix<T, NRows, NCols, Options>& matrix, const string& message_name=DefaultMessageName, const string& key=DefaultKey):
        Message(CoreModuleName, message_name, nullptr), key(key)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            serialization::serialize_eigen_matrix<T, NRows, NCols, Options>(fbb, matrix, key);
        });
    }

    static shared_ptr<EigenMessage<T, NRows, NCols, Options>> Ptr(const Eigen::Matrix<T, NRows, NCols, Options>& matrix, const string& message_name=DefaultMessageName, const string& key=DefaultKey) {
        return std::make_shared<EigenMessage<T, NRows, NCols, Options>>(matrix, message_name, key);
    }

    void print_on(ostream& os) const override {
        auto root = root_map()[this->key].AsMap();
        auto dtype = root["dtype"].AsInt8();
        os << "<EigenMessage" << " key:" << this->key 
           << " shape: ()" << NRows << ", " << NCols << ")"
           << " dtype:" << serialization::type_name_from_code(dtype) 
           << " ";
        Message::print_on(os);
        os << ">";
    }

    const Eigen::Map<const Eigen::Matrix<T, NRows, NCols, Options>> value() const {
        auto root = root_val(this->key);
        return serialization::deserialize_eigen_matrix<T, NRows, NCols, Options>(root);
    }

protected:

    string key;
};


/**
 * Carries a single xtensor-xarray buffer under some map key,
 * and also a uint64 count of the total number of values
 * seen so far in the highest dimension.
  
 */
template <typename T>
class TensorBufferMessage: public Message {
public:
    TensorBufferMessage(
        Message& other, 
        const string& buffer_key="buffer", 
        const string& count_key="count"): 
            Message(other),
            buffer_key(buffer_key),
            count_key(count_key) {}

    TensorBufferMessage(
        const xt::xarray<T>& matrix, 
        uint64_t count, 
        const string& buffer_key="buffer",
        const string& count_key="count"):
            Message(CoreModuleName, "TensorBuffer"),
            buffer_key(buffer_key),
            count_key(count_key)
    {
        flexbuffers::Builder fbb = get_builder();
        WriteMapRoot(fbb, [&](){
            fbb.UInt(count_key.c_str(), count);
            serialization::serialize_flex_array<T>(fbb, matrix, buffer_key);
        });
    }

    const serialization::flextensor_adaptor<T> buffer() const {
        auto root = root_map()[buffer_key];
        return serialization::deserialize_flex_array<T>(root);
    }

    uint64_t count() const {
        return root_map()[count_key].AsUInt64();
    }

protected:

    string buffer_key;
    string count_key;
};

constexpr char PingMessageName[] = "ping";
constexpr char PongMessageName[] = "pong";
constexpr char GetNameMessageName[] = "getname";
constexpr char GotNameMessageName[] = "gotname";
constexpr char GetGuidMessageName[] = "getguid";
constexpr char GotGuidMessageName[] = "gotguid";
constexpr char StartMessageName[] = "start";
constexpr char StopMessageName[] = "stop";
constexpr char OKMessageName[] = "ok";

} // namespace roboflex::core

#endif // ROBOFLEX_CORE_COREMESSAGES__H
