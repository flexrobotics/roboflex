#ifndef ROBOFLEX_SERIALIATION__H
#define ROBOFLEX_SERIALIATION__H

#include <string>
#include <vector>
#include <map>
#include <exception>

namespace roboflex::serialization {

using std::string, std::vector, std::map, std::runtime_error;

class Blob {
public:
    Blob(const uint8_t* data, size_t size):
        _data(data), _size(size) {}

    const uint8_t* data() const { return _data; }
    size_t size() const { return _size; }

protected:
    const uint8_t* _data;
    size_t _size;
};

template <typename T>
concept DataUnitConcept = requires (T v){
    v.is_bool();
};

template <DataUnitConcept T>
class Msg {
public:

};

class DataUnitChild {
public:
    bool is_bool() const { return false; }
};

using DMsg = Msg<DataUnitChild>;

class DataUnit {
public:
    virtual bool is_bool() = 0;
    virtual bool is_uint8() = 0;
    virtual bool is_uint16() = 0;
    virtual bool is_uint32() = 0;
    virtual bool is_uint62() = 0;
    virtual bool is_int8() = 0;
    virtual bool is_int16() = 0;
    virtual bool is_int32() = 0;
    virtual bool is_int62() = 0;
    virtual bool is_float() = 0;
    virtual bool is_double() = 0;
    virtual bool is_string() = 0;
    virtual bool is_vector() = 0;
    virtual bool is_map() = 0;
    virtual bool is_blob() = 0;

    virtual bool as_bool() = 0;
    virtual uint8_t as_uint8() = 0;
    virtual uint16_t as_uint16() = 0;
    virtual uint32_t as_uint32() = 0;
    virtual uint64_t as_uint62() = 0;
    virtual int8_t as_int8() = 0;
    virtual int16_t as_int16() = 0;
    virtual int32_t as_int32() = 0;
    virtual int64_t as_int62() = 0;
    virtual float as_float() = 0;
    virtual double as_double() = 0;
    virtual string as_string() = 0;
    virtual vector<DataUnit> as_vector() = 0;
    virtual map<string, DataUnit> as_map() = 0;
    virtual Blob as_blob() = 0;

    template <typename T>
    bool is() { return false; }

    template <typename T>
    T as() { throw runtime_error("Not implemented"); }
};

template <> bool DataUnit::is<uint8_t>() { return is_uint8(); }
template <> bool DataUnit::is<uint16_t>() { return is_uint16(); }

template <> uint8_t DataUnit::as() { return as_uint8(); }
template <> uint16_t DataUnit::as() { return as_uint16(); }

} // roboflex::serialization

#endif // ROBOFLEX_SERIALIATION__H