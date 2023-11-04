#include <thread>
#include <chrono>
#include <iostream>
#include "roboflex_core/core_nodes/universal_data_saver.h"
#include "roboflex_core/util/utils.h"

namespace roboflex {
namespace nodes {

UniversalDataSaver::UniversalDataSaver(
    const std::string& file_path,
    bool append,
    const std::string& name):
        Node(name),
        file_path(file_path)
{
    open_file_stream(append);
}

UniversalDataSaver::~UniversalDataSaver()
{
    flush();
    output_file_stream.close();
}

void UniversalDataSaver::open_file_stream(bool append)
{
    std::unique_lock<std::recursive_mutex> lck(mtx);
    auto k = std::ios::out | std::ios::binary;
    if (append) k |= std::ios::app;
    output_file_stream.open(file_path, k);
}

void UniversalDataSaver::set_file_path(const std::string& file_path_, bool append)
{
    std::unique_lock<std::recursive_mutex> lck(mtx);
    output_file_stream.close();
    file_path = file_path_;
    open_file_stream(append);
}

void UniversalDataSaver::flush()
{
    output_file_stream.flush();
}

void UniversalDataSaver::record_message(MessagePtr m)
{
    this->receive(m);
}

void UniversalDataSaver::receive(MessagePtr m)
{
    // grab the mutex now
    std::unique_lock<std::recursive_mutex> lck(mtx);

    // write total byte size using 4 bytes
    uint32_t total_bytes = m->get_raw_size();
    char s[4];
    memcpy(s, &total_bytes, 4);
    output_file_stream.write(s, 4);

    // write the data
    output_file_stream.write((const char*)m->get_raw_data(), m->get_raw_size());

    // pass it on...
    signal(m);
}

} // namespace nodes
} // namespace roboflex
