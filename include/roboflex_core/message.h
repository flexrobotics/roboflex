#ifndef ROBOFLEX_CORE_MESSAGE__H
#define ROBOFLEX_CORE_MESSAGE__H

#include "message_backing_store.h"
#include "flatbuffers/flexbuffers.h"
#include "serialization/flex_utils.h"
#include "util/uuid.h"
#include "util/utils.h"

namespace roboflex::core {

using std::string, std::shared_ptr, std::ostream, sole::uuid;

/**
 * The Message class is a message that uses FlexBuffers for serialization.
 * It adds a "_meta" attribute, which is a map containing metadata about the
 * message and the sender such as timestamp, sending node name, etc.
 */
class Message {
public:

    Message(MessageBackingStorePtr data):
        _data(data) {}

    Message(
        const string& module_name,
        const string& message_name,
        MessageBackingStorePtr data = nullptr):
            _data(data),         
            _module_name(module_name),
            _message_name(message_name) {}

    Message(Message& other, const string& child_message_name=""):
        _data(other.payload()) {
            if (!child_message_name.empty()) {
                if (other.message_name() != child_message_name) {
                    throw std::runtime_error("Expected message with name \"" + child_message_name + "\", but received \"" + other.message_name() + "\"");
                }
            }
        }

    Message(
        const Message& take_header_from,
        std::function<void(flexbuffers::Builder&)> payload_function);

    Message(
        const string& module_name,
        const string& message_name,
        const Message& copy_from,
        const std::set<string>& omit_keys,
        std::function<void(flexbuffers::Builder&)> payload_function);

    virtual ~Message() {}

    virtual void print_on(ostream& os) const;
    virtual string to_string() const;

    void set_sender_info(const std::string& name, const sole::uuid& guid, uint64_t message_send_counter);

    flexbuffers::Reference get_flex_root() const {
        return payload()->get_size() == 0 ? flexbuffers::Reference() : flexbuffers::GetRoot(payload()->get_data(), payload()->get_size());
    }

    flexbuffers::Map root_map() const {
        return get_flex_root().AsMap();
    }

    flexbuffers::Reference root_val(const string& key) const {
        // Hey, should we throw an exception if the key doesn't exist?
        // It wouldn't be very flexbuffer-like, but it would be more
        // informative to the user if something were to go wrong, such
        // as trying to deserialize a key that doesn't exist.
        return root_map()[key];
    }

    // Meta information is a vector off of the root
    // map under the key "_meta". The value is a
    // vector of values of different types, containing
    // the timestamp, message counter, source node info,
    // and so on...
    flexbuffers::Vector get_meta() const {
        return root_val("_meta").AsVector();
    }

    // Position 0: timestamp
    double timestamp() const {
        return get_meta()[0].AsDouble();
    }

    void set_timestamp(double t) {
        get_meta()[0].MutateFloat(t);
    }
    
    // Position 1: message counter
    uint64_t message_counter() const {
        return get_meta()[1].AsUInt64();
    }

    void set_message_counter(uint64_t c) {
        get_meta()[1].MutateInt(c);
    }
    
    // Position 2: source node guid
    const uuid source_node_guid() const {
        auto blob = get_meta()[2].AsBlob();
        return roboflex::serialization::deserialize_uuid(blob);
    }

    void set_source_node_guid(const uuid& g) {

        // blit data directly into the flexbuffer blob, in memory

        char guidchars[16];
        memcpy(guidchars, &(g.ab), 8);
        memcpy(guidchars+8, &(g.cd), 8);

        auto data_portion = get_meta()[2].AsBlob();
        const uint8_t* const_data = data_portion.data();
        uint8_t* data = const_cast<uint8_t*>(const_data);

        memcpy((void*)data, (const void*)guidchars, 16);
    }
     
    // Position 3: source node name
    const string source_node_name() const {
        auto s = std::string(get_meta()[3].AsString().str());
        size_t first_zero_pos = s.find_first_of(char(0));
        if (first_zero_pos != std::string::npos) {
            s.erase(first_zero_pos, s.size());
        }
        return s;
    }

    void set_source_node_name(const string& n) {
        if (n.length() > 32) {
            throw std::runtime_error("You tried to set source_node_name to a string length > 32, and that is verboten. (\"" + n + "\")");
        }
        char c[32] = {};
        memcpy(c, n.c_str(), n.length());
        get_meta()[3].MutateString(c, 32);
    }

    // Position 4: module name
    const string module_name() const {
        return get_meta()[4].AsString().str();
    }

    // Position 5: message name
    const string message_name() const {
        return get_meta()[5].AsString().str();
    }

    const MessageBackingStorePtr payload() const { return _data; }

    // Get the actual active bytes and size
    uint8_t* get_data() { return payload() == nullptr ? nullptr : payload()->get_data(); }
    uint32_t get_size() const { return payload() == nullptr ? 0 : payload()->get_size(); }

    uint8_t* get_raw_data() { return payload() == nullptr ? nullptr : payload()->get_raw_data(); }
    uint32_t get_raw_size() const { return payload() == nullptr ? 0 : payload()->get_raw_size(); }

    // These are common to all messages, not just flex messages. The "message announce"
    // is the 4 bytes of the header. For flex messages (currently the only message supported),
    // this value is "RFLX".
    std::string_view message_announce() const { return payload()->message_announce(); }
    // The message size is the size of the message, including the header, encoded in 
    // the next four bytes. It should be == to get_raw_size()
    uint32_t message_size() const { return payload()->message_size(); }
    // ... after that, all data is encoded in Flexbuffers.

protected:

    // If children want to instantiate themselves, they need to get
    // a builder USING THIS METHOD!!! instead of instantiating 
    // their own builder.
    flexbuffers::Builder get_builder();

    template <typename F>
    void WriteMapRoot(
        flexbuffers::Builder& fbb,
        F f,
        bool create_meta = true)
    {
        char empty_name[33] = {};
        char empty_guid[16] = {};

        fbb.Map([&]() {

            if (create_meta) {
                fbb.Vector("_meta", [&]() {
                    fbb.Double(get_current_time());
                    fbb.UInt(std::numeric_limits<uint64_t>::max());
                    fbb.Blob(empty_guid, 16);
                    fbb.String(empty_name, 32);
                    fbb.String(_module_name);
                    fbb.String(_message_name);
                });
            }

            // write the payload via the provided function
            f();
        });

        finish_serialization(fbb);
    }

private:

    void finish_serialization(flexbuffers::Builder& fbb);

    MessageBackingStorePtr _data;
    string _module_name;
    string _message_name;
};

using MessagePtr = shared_ptr<Message>;

inline std::ostream& operator<< (std::ostream& os, const Message& m)
{
    m.print_on(os);
    return os;
}

} // roboflex::core

#endif // ROBOFLEX_CORE_MESSAGE__H
