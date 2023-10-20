#include <sstream>
#include "roboflex_core/message.h"

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

void copy_flex(flexbuffers::Builder& fbb, const flexbuffers::Reference& vr) 
{
    flexbuffers::Type value_type = vr.GetType();
    switch (value_type) {
    case flexbuffers::FBT_NULL:
        fbb.Null();
        break;
    case flexbuffers::FBT_INT:
        fbb.Add(vr.AsInt64());
        break;
    case flexbuffers::FBT_UINT:
        fbb.Add(vr.AsUInt64());
        break;
    case flexbuffers::FBT_FLOAT:
        fbb.Add(vr.AsDouble());
        break;
    case flexbuffers::FBT_KEY:
        fbb.Add(vr.AsKey());
        break;
    case flexbuffers::FBT_STRING:
        fbb.Add(vr.AsString());
        break;
    case flexbuffers::FBT_INDIRECT_INT:
        fbb.Add(vr.AsInt64());
        break;
    case flexbuffers::FBT_INDIRECT_UINT:
        fbb.Add(vr.AsUInt64());
        break;
    case flexbuffers::FBT_INDIRECT_FLOAT:
        fbb.Add(vr.AsDouble());
        break;
    case flexbuffers::FBT_MAP:
        fbb.Map([&](){
            auto map = vr.AsMap();
            auto keys = map.Keys();
            for (size_t i=0; i<keys.size(); i++) {
                string key = keys[i].AsString().str();
                fbb.Key(key);
                copy_flex(fbb, map[key]);
            }
        });

        break;
    case flexbuffers::FBT_VECTOR:
        fbb.Vector([&](){
            auto vec = vr.AsVector();
            for (size_t i=0; i<vec.size(); i++) {
                copy_flex(fbb, vec[i]);
            }
        });
        break;
    case flexbuffers::FBT_VECTOR_INT:
    case flexbuffers::FBT_VECTOR_UINT:
    case flexbuffers::FBT_VECTOR_FLOAT:
    case flexbuffers::FBT_VECTOR_BOOL:
        fbb.TypedVector([&](){
            auto vec = vr.AsTypedVector();
            for (size_t i=0; i<vec.size(); i++) {
                copy_flex(fbb, vec[i]);
            }
        });
        break;

    case flexbuffers::FBT_VECTOR_KEY:
        fbb.Vector([&](){
            auto vec = vr.AsVector();
            for (size_t i=0; i<vec.size(); i++) {
                copy_flex(fbb, vec[i]);
            }
        });
        break;

    case flexbuffers::FBT_VECTOR_STRING_DEPRECATED:
        break;

    case flexbuffers::FBT_VECTOR_INT2: 
        {
        auto ftv = vr.AsFixedTypedVector();
        int64_t v[2] = { ftv[0].AsInt64(), ftv[1].AsInt64()};
        fbb.FixedTypedVector(v, 2);
        }
        break;
    case flexbuffers::FBT_VECTOR_UINT2:
        {
        auto ftv = vr.AsFixedTypedVector();
        uint64_t v[2] = {ftv[0].AsUInt64(), ftv[1].AsUInt64()};
        fbb.FixedTypedVector(v, 2);
        }
        break;
    case flexbuffers::FBT_VECTOR_FLOAT2:
        {
        auto ftv = vr.AsFixedTypedVector();
        double v[2] = {ftv[0].AsDouble(), ftv[1].AsDouble()};
        fbb.FixedTypedVector(v, 2);
        }
        break;
    case flexbuffers::FBT_VECTOR_INT3:
        {
        auto ftv = vr.AsFixedTypedVector();
        int64_t v[3] = {ftv[0].AsInt64(), ftv[1].AsInt64(), ftv[2].AsInt64()};
        fbb.FixedTypedVector(v, 3);
        }
        break;
    case flexbuffers::FBT_VECTOR_UINT3:
        {
        auto ftv = vr.AsFixedTypedVector();
        uint64_t v[3] = {ftv[0].AsUInt64(), ftv[1].AsUInt64(), ftv[2].AsUInt64()};
        fbb.FixedTypedVector(v, 3);
        }
        break;
    case flexbuffers::FBT_VECTOR_FLOAT3:
        {
        auto ftv = vr.AsFixedTypedVector();
        double v[3] = {ftv[0].AsDouble(), ftv[1].AsDouble(), ftv[2].AsDouble()};
        fbb.FixedTypedVector(v, 3);
        }
        break;
    case flexbuffers::FBT_VECTOR_INT4:
        {
        auto ftv = vr.AsFixedTypedVector();
        int64_t v[4] = {ftv[0].AsInt64(), ftv[1].AsInt64(), ftv[2].AsInt64(), ftv[3].AsInt64()};
        fbb.FixedTypedVector(v, 4);
        }
        break;
    case flexbuffers::FBT_VECTOR_UINT4:
        {
        auto ftv = vr.AsFixedTypedVector();
        uint64_t v[4] = {ftv[0].AsUInt64(), ftv[1].AsUInt64(), ftv[2].AsUInt64(), ftv[3].AsUInt64()};
        fbb.FixedTypedVector(v, 4);
        }
        break;
    case flexbuffers::FBT_VECTOR_FLOAT4:
        {
        auto ftv = vr.AsFixedTypedVector();
        double v[4] = {ftv[0].AsDouble(), ftv[1].AsDouble(), ftv[2].AsDouble(), ftv[3].AsDouble()};
        fbb.FixedTypedVector(v, 4);
        }
        break;
    case flexbuffers::FBT_BLOB: 
        fbb.Blob(vr.AsBlob().data(), vr.AsBlob().size());
        break;
    case flexbuffers::FBT_BOOL:
        fbb.Add(vr.AsBool());
        break;
    // default:
    //     break;
    }
}

Message::Message(
    const string& module_name,
    const string& message_name,
    const Message& copy_from,
    const std::set<string>& omit_keys,
    std::function<void(flexbuffers::Builder&)> payload_function):
        Message(module_name, message_name)
{
    auto copy_from_root = copy_from.root_map();
    flexbuffers::Builder fbb = get_builder();

    WriteMapRoot(fbb, [&]() {

        // copy all the top-level keys, except for the ones we should omit
        auto keys = copy_from_root.Keys();
 
        for (size_t i=0; i<keys.size(); i++) {
            string key = keys[i].AsString().str();
            if (!omit_keys.contains(key) && key != "_meta") {
                fbb.Key(key);
                copy_flex(fbb, copy_from_root[key]);
            } else {
                //std::cout << "---------- SKIPPING " << key << std::endl;
            }
        }

        char empty_name[33] = {};
        char empty_guid[16] = {};

        fbb.Vector("_meta", [&]() {
            fbb.Double(copy_from.timestamp());
            fbb.UInt(copy_from.message_counter());
            fbb.Blob(empty_guid, 16);
            fbb.String(empty_name, 32);
            fbb.String(module_name);
            fbb.String(message_name);
        });

        // Finally, call the payload function to write whatever the user wants
        payload_function(fbb);

    }, false);

    //set_source_node_name(copy_from.source_nod)
}

void Message::print_on(ostream& os) const
{
    os << "<Message \"" << module_name() << "::" << message_name() << "\""
       << " t: " << std::fixed << std::setprecision(3) << timestamp()
       << " #: " << message_counter()
       << " source: \"" << source_node_name() << "\"|" << source_node_guid()
       << " payload: " << payload()->get_size() << " bytes, top-level keys: [";

    // print out the top-level keys
    auto keys = root_map().Keys();
    for (size_t i=0; i<keys.size(); i++) {
        string key = keys[i].AsString().str();
        if (i != 0) {
            os << ", ";
        }
        os << key;
    }

    os << "] <" << message_announce() << "|" << message_size() << ">"
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
    }

    if (source_node_guid().ab == 0 && source_node_guid().cd == 0) {
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
