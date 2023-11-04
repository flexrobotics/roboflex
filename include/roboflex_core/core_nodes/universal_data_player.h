#ifndef ROBOFLEX_UNIVERSAL_DATA_PLAYER__H
#define ROBOFLEX_UNIVERSAL_DATA_PLAYER__H

#include <iostream>
#include <fstream>
#include <mutex>
#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

using std::string;

/**
 * A node that plays what UniversalDataSaver writes,
 * optionally repeating and optionally in experienced realtime
 * (as opposed to as-fast-as-can-be-read-from-file).
 */
class UniversalDataPlayer: public RunnableNode {
public:
    UniversalDataPlayer(
        const string& file_path,
        const string& name = "UniversalDataPlayer",
        bool forever = false,
        bool realtime = true,
        bool rewrite_timestamps = false,
        bool verbose = true);

    virtual ~UniversalDataPlayer();

    bool produce();
    void produce_all_once();

    const string & get_file_path() const { return file_path; }
    bool get_forever() const { return forever; }
    bool get_realtime() const { return realtime; }
    bool get_rewrite_timestamps() const { return rewrite_timestamps; }
    bool get_verbose() const { return verbose; }

    void child_thread_fn() override;

protected:
    uint32_t read_and_signal();
    void reset_time();
    void reset_production();

    string file_path;
    std::ifstream input_file_stream;
    bool forever;
    bool realtime;
    bool rewrite_timestamps;
    bool verbose;

    unsigned int num_messages_replayed;
    unsigned long int num_bytes_replayed;
    double replay_t0;
    double messages_t0;
};


} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_UNIVERSAL_DATA_PLAYER__H
