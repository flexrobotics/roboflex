#include <sstream>
#include "message.h"

using std::stringstream;

namespace roboflex::core {

Message::Message(
    const Message& take_header_from,
    std::function<void(flexbuffers::Builder&)> payload_function):
        Message(take_header_from.module_name(), take_header_from.message_name())
{
    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&](){ payload_function(fbb); });
    set_timestamp(take_header_from.timestamp());
    set_source_node_name(take_header_from.source_node_name());
    set_source_node_guid(take_header_from.source_node_guid());
    set_message_counter(take_header_from.message_counter());
}

void Message::print_on(ostream& os) const
{
    os << "<Message \"" << module_name() << "::" << message_name() << "\""
       << " t: " << std::fixed << std::setprecision(3) << timestamp()
       << " source: \"" << source_node_name() << "\"|" << source_node_guid()
       << " #: " << message_counter()
       << " payload: " << payload()->get_size() << " bytes"
       << " <" << message_announce() << "|" << message_size() << ">"
       << ">";
}

string Message::to_string() const 
{
    stringstream sst;
    sst << *this;
    return sst.str();
}

void Message::set_sender_info(const std::string& name, const sole::uuid& guid, uint64_t message_send_counter)
{
    if (message_counter() == std::numeric_limits<uint64_t>::max()) {
        set_message_counter(message_send_counter);
        set_source_node_guid(guid);
        set_source_node_name(name);
    }
}

flexbuffers::Builder Message::get_builder() 
{
    // Create a flex-buffer builder
    flexbuffers::Builder fbb;

    // make sure it has enough memory to contain at least the
    // fixed-header size
    fbb.ExtendBuffer(MESSAGE_HEADER_SIZE);

    return fbb;
}

void Message::finish_serialization(flexbuffers::Builder& fbb) 
{
    // Hey, want to see what's going on? Uncomment this stuff...
    //std::cout << "msg: " << this->to_string() << std::endl;

    // mandatory, apparently
    fbb.Finish();

    // Get the buffer from the flexbuffer Builder. It will be a const ref.
    const std::vector<uint8_t> & bf = fbb.GetBuffer();

    // Cast away const-ness, aim at foot...
    std::vector<uint8_t>& nonconst_bf = const_cast<std::vector<uint8_t> &>(bf);

    // These should be the same
    //std::cout << "bf:           " << (void*)(bf.data()) << "  " << bf.size() << std::endl;
    //std::cout << "nonconst_bf:  " << (void*)(nonconst_bf.data()) << "  " << nonconst_bf.size() << std::endl;

    // Move ownership into my payload.
    auto payload = make_shared<MessageBackingStoreVector>(std::move(nonconst_bf));
    this->_data = payload;

    // Now witness how both bf and nonconst_bf's values are null. Payload has taken over.
    //std::cout << "payload:      " << (void*)(payload->vec_bytes.data()) << "  " << payload->vec_bytes.size() << std::endl;
    //std::cout << "bf:           " << (void*)(bf.data()) << "  " << bf.size() << std::endl;
    //std::cout << "nonconst_bf:  " << (void*)(nonconst_bf.data()) << "  " << nonconst_bf.size() << std::endl;
    //exit(0);

    // finally, blit the header: RFLXSIZE
    this->_data->blit_header();
}

} // namespace roboflex::core
