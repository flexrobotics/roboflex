#ifndef ROBOFLEX_UNIVERSAL_DATA_SAVER__H
#define ROBOFLEX_UNIVERSAL_DATA_SAVER__H

#include <iostream>
#include <fstream>
#include <mutex>
#include "roboflex_core/node.h"

namespace roboflex {
using namespace core;
namespace nodes {

using std::string;

/**
 * A node that just appends raw message data to file.
 */
class UniversalDataSaver: public Node {
public:
    UniversalDataSaver(
        const string& file_path,
        bool append = true,
        const string& name = "UniversalDataSaver");
    virtual ~UniversalDataSaver();

    const string & get_file_path() const { return file_path; }
    void set_file_path(const string& file_path_, bool append=true);
    void flush();
    void record_message(MessagePtr m);

    void receive(MessagePtr m) override;

protected:
    void open_file_stream(bool append = true);

    string file_path;
    std::ofstream output_file_stream;
    std::recursive_mutex mtx;
};

} // namespace nodes
} // namespace roboflex

#endif // ROBOFLEX_UNIVERSAL_DATA_SAVER__H