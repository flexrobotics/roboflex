#include "png.h"
#include "roboflex/core/core_messages/core_messages.h"
#include "lodepng/lodepng.h"

namespace roboflex {
namespace utilpng {


// -- JPEGImage --

PNGImage::PNGImage(void* data, int bytes, const string& key, core::Message& from, const std::set<string>& omit_keys):
    Message(ModuleName, MessageName, from, omit_keys, [&](flexbuffers::Builder& fbb) {
        fbb.Key(key);
        fbb.Blob(data, bytes);
    }),
    key(key)
{

}

void PNGImage::print_on(ostream& os) const
{
    os << "<PNGImage"
       << "  bytes: " << get_data_size_bytes()
       << "  data: " << static_cast<const void*>(get_data()) << " ";
    Message::print_on(os);
    os << ">";
}


// -- PNGCompressor --

PNGCompressor::PNGCompressor(const string& image_key, const string& output_key, const string& filename_prefix, const string& name, bool debug):
    core::Node(name),
    image_key(image_key),
    output_key(output_key),
    filename_prefix(filename_prefix),
    debug(debug),
    compression_buffer_size(640 * 480 * 3),
    compression_buffer(malloc(compression_buffer_size))
{

}

PNGCompressor::~PNGCompressor() 
{
    std::cerr << "~PNGCompressor" << std::endl;
    free(compression_buffer);
}

void PNGCompressor::receive(core::MessagePtr m) {

    // Get the image (zero-copy)
    xt::xtensor<uint8_t, 3> rgb_tensor = serialization::deserialize_flex_tensor<uint8_t, 3>(m->root_map()[this->image_key], false);

    int height = rgb_tensor.shape()[0];
    int width = rgb_tensor.shape()[1];

    const uint8_t *pImage_data = rgb_tensor.data();

    // EITHER write to file or write to and signal a message
    if (filename_prefix.size() > 0) {

        string filename = filename_prefix + std::to_string(m->timestamp()) + ".png";

        LodePNGColorType colortype = LCT_RGB;
        unsigned bitdepth = 8;

        unsigned error_code = lodepng::encode(
            filename,
            pImage_data,
            width, 
            height,
            colortype, 
            bitdepth);

        if (error_code != 0) {
            std::cerr << "PNGCompressor::receive failed to call lodepng::encode (to file), height=" << height << " width=" << width << " -> " << error_code << std::endl;
        } else {
            if (debug) {
                std::cerr << "PNGCompressor::receive wrote to file \"" << filename << "\", height=" << height << " width=" << width << std::endl;
            }
        }

    } else {

        std::vector<unsigned char> out;
        LodePNGColorType colortype = LCT_RGB;
        unsigned bitdepth = 8;

        unsigned error_code = lodepng::encode(
            out,
            pImage_data, 
            width, 
            height,
            colortype, 
            bitdepth);

        if (error_code != 0) {
            std::cerr << "PNGCompressor::receive failed to call lodepng::encode, height=" << height << " width=" << width << " ->  " << error_code << std::endl;
        } else {
            auto o = std::make_shared<PNGImage>(out.data(), out.size(), this->output_key, *m, std::set{this->image_key});
            
            // Or, we can do it without a sub-class
            // auto o = std::make_shared<core::Message>("util_jpeg", "JPEGImage", *m, std::set{this->image_key}, [&](flexbuffers::Builder& fbb) {
            //     fbb.Key(this->output_key);
            //     fbb.Blob(compression_buffer, buf_size);
            // });

            if (debug) {
                int total_bytes = height * width * 3;
                std::cerr << "PNGCompressor::receive wrote to memory: " << out.size() << " bytes" << 
                    ", from height=" << height << 
                    ", width=" << width << 
                    ", total bytes=" << total_bytes << 
                    ", compressed to=" << std::fixed << std::setprecision(3) << ((float)out.size() / total_bytes * 100.0) << "\%" << 
                    " -> signalling " << o->to_string() << std::endl;
            }
            
            this->signal(o);
        }
    }
}


// -- PNGDecompressor --

PNGDecompressor::PNGDecompressor(const string& input_key, const string& output_key, const string& name, bool debug):
    core::Node(name),
    input_key(input_key),
    output_key(output_key),
    debug(debug)
{

}

void PNGDecompressor::receive(core::MessagePtr m) {

    auto jpeg_blob = m->root_map()[this->input_key].AsBlob();
    const uint8_t* jpeg_raw_data = jpeg_blob.data();
    int jpeg_raw_data_size = jpeg_blob.size();

    unsigned width = 0;
    unsigned height = 0;
 
    LodePNGColorType colortype = LCT_RGB;
    unsigned bitdepth = 8;
    std::vector<unsigned char> out;

    unsigned error_code = lodepng::decode(
        out, 
        width, 
        height,
        jpeg_raw_data,
        jpeg_raw_data_size,
        colortype, 
        bitdepth);

    if (error_code != 0) {

        std::cerr << "PNGDecompressor::receive FAILED to call decode" << std::endl;
    
    } else {

        std::vector<size_t> vshape = std::vector<size_t>{(size_t)height, (size_t)width, 3};
        size_t total_bytes = width * height * 3;
        if (total_bytes != out.size()) {
            throw std::runtime_error("PNGDecompressor::receive total_bytes(" + std::to_string(total_bytes) + ") != out.size() (" + std::to_string(out.size()) + ")");
        }
        auto tensor = xt::adapt(out.data(), total_bytes, xt::no_ownership(), vshape);
        
        // Get a copy of the original message, omit the jpeg key/value, and insert the rgb
        auto o = std::make_shared<core::Message>(ModuleName, "PNGImage", *m, std::set{this->input_key}, [&](flexbuffers::Builder& fbb) {
            serialization::serialize_flex_tensor<uint8_t, 3>(fbb, tensor, this->output_key);
        });

        if (debug) {
            std::cerr << "PNGDecompressor::receive decompressed into height=" << height << ", width=" << width << " -> " << o->to_string() << std::endl;
        }

        this->signal(o);
    }
}

} // namespace utilpng
} // namespace roboflex
