#include <sstream>
#include <iostream>
#include "roboflex_core/message_backing_store.h"


namespace roboflex::core {


// -- MessageBackingStore --

void MessageBackingStore::raw_data_deletion_function(void */*data*/, void *hint)
{
    std::shared_ptr<MessageBackingStore>* p = (std::shared_ptr<MessageBackingStore>*)hint;
    delete p;
}

std::string MessageBackingStore::to_string() const
{
    std::stringstream sst;
    sst << *this;
    return sst.str();
}

void MessageBackingStore::blit_header() 
{
    uint8_t* message_start = (uint8_t*)(get_data()) - MESSAGE_HEADER_SIZE;
    memcpy(message_start, ROBOFLEX_FLEX_MESSAGE_FORMAT_HEADER, 4);
    uint32_t size = get_size() + MESSAGE_HEADER_SIZE;
    memcpy(message_start + 4, &size, 4);
}

const std::string_view MessageBackingStore::message_announce() 
{
    return std::string_view((char*)get_data() - MESSAGE_HEADER_SIZE, 4);
}

uint32_t MessageBackingStore::message_size() const 
{
    const uint8_t* szpos = get_data() - MESSAGE_HEADER_SIZE + 4;
    return (uint32_t)(szpos[3] << 24 | szpos[2] << 16 | szpos[1] << 8 | szpos[0]);
}


// -- MessageBackingStoreVector --

void MessageBackingStoreVector::print_on(ostream& os) const
{
    os << "<MessageBackingStoreVector"
       << " from: " << (const void*)(&(this->vec_bytes[0]))
       << " to: " << (const void*)(&(this->vec_bytes[this->get_size()-1]))
       << " bytes: " << this->get_size()
       << ">";
}


// -- MessageBackingStoreNew --

void MessageBackingStoreNew::print_on(ostream& os) const
{
    os << "<MessageBackingStoreNew"
       << " data: " << static_cast<const void*>(this->data)
       << " size: " << this->get_size()
       << ">";
}

} // namespace roboflex::core
