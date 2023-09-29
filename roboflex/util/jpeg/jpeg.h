#ifndef ROBOFLEX_UTIL_JPEG__H
#define ROBOFLEX_UTIL_JPEG__H

#include "roboflex/core/core.h"

namespace roboflex {
namespace utiljpeg {

constexpr char ModuleName[] = "util_jpeg";

struct JPEGImage : public core::Message {
    
    inline static const char MessageName[] = "JPEGImage";

    JPEGImage(core::Message& other): Message(other) {}
    JPEGImage(void* data, int bytes, const string& key, core::Message& from, const std::set<string>& omit_keys);

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

class JPEGCompressor: public core::Node {
public:
    JPEGCompressor(
        const string& image_key = "rgb",
        const string& output_key = "jpeg",
        const string& filename_prefix = "",
        const string& name = "JPEGCompressor",
        bool debug = false);

    virtual ~JPEGCompressor();

    void receive(core::MessagePtr m) override;

protected:

    string image_key;
    string output_key;
    string filename_prefix;
    bool debug;

    int compression_buffer_size;
    void * compression_buffer;
};

class JPEGDecompressor: public core::Node {
public:
    JPEGDecompressor(
        const string& input_key = "jpeg",
        const string& output_key = "rgb",
        const string& name = "JPEGDecompressor",
        bool debug = false);

    void receive(core::MessagePtr m) override;

protected:

    string input_key;
    string output_key;
    bool debug;
};

} // namespace utiljpeg
} // namespace roboflex

#endif // ROBOFLEX_UTIL_JPEG__H

