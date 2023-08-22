#ifndef ROBOFLEX_UTIL_PNG__H
#define ROBOFLEX_UTIL_PNG__H

#include "core/core.h"

namespace roboflex {
namespace utilpng {

constexpr char ModuleName[] = "util_png";

struct PNGImage : public core::Message {
    
    inline static const char MessageName[] = "PNGImage";

    PNGImage(core::Message& other): Message(other) {}
    PNGImage(void* data, int bytes, const string& key, core::Message& from, const std::set<string>& omit_keys);

    const uint8_t* get_data() const {
        auto data_portion = root_map()[key].AsBlob();
        const uint8_t* const_data = data_portion.data();
        return const_data;
    }

    size_t get_data_size_bytes() const {
        auto data_portion = root_map()[key].AsBlob();
        return data_portion.size();
    }

    void print_on(ostream& os) const override;

protected:

    string key;
};

class PNGCompressor: public core::Node {
public:
    PNGCompressor(
        const string& image_key = "rgb",
        const string& output_key = "png",
        const string& filename_prefix = "",
        const string& name = "PNGCompressor",
        bool debug = false);

    virtual ~PNGCompressor();

    void receive(core::MessagePtr m) override;

protected:

    string image_key;
    string output_key;
    string filename_prefix;
    bool debug;

    int compression_buffer_size;
    void * compression_buffer;
};

class PNGDecompressor: public core::Node {
public:
    PNGDecompressor(
        const string& input_key = "png",
        const string& output_key = "rgb",
        const string& name = "PNGDecompressor",
        bool debug = false);

    void receive(core::MessagePtr m) override;

protected:

    string input_key;
    string output_key;
    bool debug;
};

} // namespace utilpng
} // namespace roboflex

#endif // ROBOFLEX_UTIL_PNG__H

