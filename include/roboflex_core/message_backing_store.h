#ifndef ROBOFLEX_CORE_MESSAGE_BACKING_STORE__H
#define ROBOFLEX_CORE_MESSAGE_BACKING_STORE__H

#include <cstring>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>

namespace roboflex::core {

using std::ostream, std::shared_ptr, std::vector;

/**
 * To start with, a little class cluster: MessageBackingStore defines an interface
 * to get raw bytes, and subclasses can account for different ways that clients
 * might own that data.
 *
 * Why does this exist at all? Because it makes it easier to perform 0-copy
 * using zmq and other systems. We can let, for instance, the zmq threads
 * delete the memory from whence it came.
 * 
 * The MessageBackingStore also knows how to blit the header into the start
 * of the data buffer.
 */
const uint32_t MESSAGE_HEADER_SIZE = 8;

constexpr char ROBOFLEX_FLEX_MESSAGE_FORMAT_HEADER[] = "RFLX";

/**
 * A message serialization just defines an interface
 * to get data to/from the wire. Why the class hierarchy?
 * Because it aids in destruction of data from different
 * sources, which helps with 0-copy.
 */
struct MessageBackingStore
{
    virtual ~MessageBackingStore() {}

    virtual void print_on(ostream& os) const = 0;
    std::string to_string() const;

    // Used for 0copy in some transport layers, such as zmq.
    // The constructor of zmq message takes an optional 'free function', and
    // optional hint. We cast a newly constructed shared pointer to the
    // message serialization as the hint and this function as the free function.
    // This function then casts the hint back to a shared pointer to
    // a message serialization, and deletes it. In this way, we keep
    // ownership of the serialization until we can truly delete it.
    static void raw_data_deletion_function(void *data, void *hint);

    // blits the 8 character header:
    // 4 bytes announce: currently only "RFLX" and 
    // 4 bytes size: the size of the message in uint32_t 
    // into the start of the data buffer.
    void blit_header();

    // The first 4 bytes of the message, which should be "RFLX".
    const std::string_view message_announce();

    // The size of the total message as encoded in the next 4 bytes,
    // which should be == the size of the data + 8.
    uint32_t message_size() const;

    // NOTE: this is a non-const pointer to the actual data, not a copy!
    // DO NOT DELETE IT! This is the pointer to the data portion, which 
    // starts 8 bytes after the message start, and which is currently
    // encoded in flexbuffer.
    uint8_t* get_data() { return get_raw_data() + MESSAGE_HEADER_SIZE; }
    const uint8_t* get_data() const { return get_raw_data() + MESSAGE_HEADER_SIZE; }
    uint32_t get_size() const { return get_raw_size() - MESSAGE_HEADER_SIZE; }
    
    virtual uint8_t* get_raw_data() = 0;
    virtual const uint8_t* get_raw_data() const = 0;
    virtual uint32_t get_raw_size() const = 0;
};

inline std::ostream& operator<< (ostream& os, const MessageBackingStore& m)
{
    m.print_on(os);
    return os;
}

using MessageBackingStorePtr = shared_ptr<MessageBackingStore>;

// The c++-y way to construct a MessageBackingStore
struct MessageBackingStoreVector: public MessageBackingStore
{
    MessageBackingStoreVector(vector<uint8_t> && bytes):
        vec_bytes(bytes) {}

    MessageBackingStoreVector(const uint8_t* bytes, size_t length):
        vec_bytes(bytes, bytes+length) {}

    virtual ~MessageBackingStoreVector() {}

    uint8_t* get_raw_data() override { return vec_bytes.data(); }
    const uint8_t* get_raw_data() const override { return vec_bytes.data(); }
    uint32_t get_raw_size() const override { return vec_bytes.size(); }

    void print_on(ostream& os) const override;

    vector<uint8_t> vec_bytes;
};

// Assumes you allocated bytes with 'new': will call 'delete data' in destructor
struct MessageBackingStoreNew: public MessageBackingStore
{
    MessageBackingStoreNew(uint8_t* data, size_t size):
        data(data), size(size) {}

    virtual ~MessageBackingStoreNew() { delete data; }

    uint8_t* get_raw_data() override { return data; }
    const uint8_t* get_raw_data() const override { return data; }
    uint32_t get_raw_size() const override { return size; }

    void print_on(ostream& os) const override;

    uint8_t * data;
    size_t size;
};

} // namespace roboflex::core

#endif // ROBOFLEX_CORE_MESSAGE_BACKING_STORE__H
