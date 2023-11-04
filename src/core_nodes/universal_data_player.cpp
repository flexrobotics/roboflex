#include <chrono>
#include <iostream>
#include <thread>
#include "roboflex_core/core_nodes/universal_data_player.h"
#include "roboflex_core/util/utils.h"

namespace roboflex {
namespace nodes {

UniversalDataPlayer::UniversalDataPlayer(
    const std::string& file_path,
    const std::string& name,
    bool forever,
    bool realtime,
    bool rewrite_timestamps,
    bool verbose):
        RunnableNode(name),
        file_path(file_path),
        forever(forever),
        realtime(realtime),
        rewrite_timestamps(rewrite_timestamps),
        verbose(verbose),
        num_messages_replayed(0),
        num_bytes_replayed(0),
        replay_t0(0),
        messages_t0(0)
{
    input_file_stream.open(file_path, std::ios::in | std::ios::binary);
    if (!input_file_stream.is_open()) {
        throw std::runtime_error("UniversalDataPlayer unable to open file \"" + file_path + "\"");
    }
}

UniversalDataPlayer::~UniversalDataPlayer()
{
    input_file_stream.close();
}

void UniversalDataPlayer::reset_time()
{
    replay_t0 = roboflex::get_current_time();
}

void UniversalDataPlayer::reset_production()
{
    // num_messages_replayed = 0;
    // num_bytes_replayed = 0;
    messages_t0 = 0;
    reset_time();
}

uint32_t UniversalDataPlayer::read_and_signal()
{
    // read the size
    char sizebuf[4];
    input_file_stream.read(sizebuf, 4);
    uint32_t size;
    memcpy(&size, sizebuf, 4);

    num_bytes_replayed += size;

    // read in size bytes
    char * buf = new char[size];
    input_file_stream.read(buf, size);

    // read the message header
    // MessageHeader header((const uint8_t*) buf, size);

    // get a serialization object that will, ultimately, delete the body_message
    //auto payload = std::make_shared<supercore::MessageBackingStoreNew>((uint8_t*)buf, size, header.get_total_size_bytes());
    auto payload = std::make_shared<core::MessageBackingStoreNew>((uint8_t*)buf, size);

    // create a message
    //auto message = std::make_shared<core::Message>(header, payload);
    auto message = std::make_shared<core::Message>(payload);

    // simulate real-time by sleeping for a bit...
    if (realtime) {

        // get the timestamp of the message, so we can wait a bit
        double message_timestamp = message->timestamp();
        double time_now = core::get_current_time();

        if (messages_t0 == 0) {
            messages_t0 = message_timestamp;
        } else {
            double elapsed_time = time_now - replay_t0;
            double time_to_wait = message_timestamp - messages_t0;
            int ms_to_sleep = (int)((time_to_wait - elapsed_time) * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms_to_sleep));
        }
    }

    if (rewrite_timestamps) {
        message->set_timestamp(core::get_current_time());
        message->set_message_counter(this->message_send_counter);
    }

    // signal observers
    this->signal(message);

    return size;
}

bool UniversalDataPlayer::produce()
{
    if (replay_t0 == 0) {
        reset_production();
    }

    if (input_file_stream) {
        if (input_file_stream.peek() == EOF) {
            if (forever) {
                input_file_stream.clear();
                input_file_stream.seekg(0, std::ios::beg);
            } else {
                reset_production();
                return false;
            }
        }

        read_and_signal();
        return true;
    }

    reset_production();
    return false;
}

void UniversalDataPlayer::produce_all_once()
{
    if (verbose) {

        double t0 = core::get_current_time();

        while (produce()) {}

        double t1 = core::get_current_time();
        std::cout << get_name() << " replayed " << message_send_counter
            << " messages (" << num_bytes_replayed << " bytes) in "
            << (t1 - t0) << " seconds" << std::endl;

    } else {
        while (produce()) {}
    }
}

void UniversalDataPlayer::child_thread_fn()
{
    reset_production();

    while (!this->stop_requested()) {

        while (!this->stop_requested() && input_file_stream && input_file_stream.peek() != EOF) {
            num_bytes_replayed += read_and_signal();
            num_messages_replayed += 1;
        }

        if (forever) {

            reset_time();

            // reset and go again
            input_file_stream.clear();
            input_file_stream.seekg(0, std::ios::beg);

        } else {

            double t1 = core::get_current_time();

            if (verbose) {
                std::cout << "DONE! Replayed " << num_messages_replayed
                    << " messages (" << num_bytes_replayed << " bytes) in "
                    << (t1 - replay_t0) << " seconds" << std::endl;
            }

            this->request_stop();
        }
    }
}


} // namespace nodes
} // namespace roboflex
