#include "jpeg.h"
#include "roboflex/core/core_messages/core_messages.h"
#include "jpeg-compressor/jpge.h" // from jpeg-compressor
#include "jpeg-compressor/jpgd.h" // from jpeg-compressor

namespace roboflex {
namespace utiljpeg {


// -- JPEGImage --

JPEGImage::JPEGImage(void* data, int bytes, const string& key, core::Message& from, const std::set<string>& omit_keys):
    Message(ModuleName, MessageName, from, omit_keys, [&](flexbuffers::Builder& fbb) {
        fbb.Key(key);
        fbb.Blob(data, bytes);
    }),
    key(key)
{

}

void JPEGImage::print_on(ostream& os) const
{
    os << "<JPEGImage"
       << "  bytes: " << get_data_size_bytes()
       << "  data: " << static_cast<const void*>(get_data()) << " ";
    Message::print_on(os);
    os << ">";
}


// -- JPEGCompressor --

JPEGCompressor::JPEGCompressor(const string& image_key, const string& output_key, const string& filename_prefix, const string& name, bool debug):
    core::Node(name),
    image_key(image_key),
    output_key(output_key),
    filename_prefix(filename_prefix),
    debug(debug),
    compression_buffer_size(640 * 480 * 3),
    compression_buffer(malloc(compression_buffer_size))
{

}

JPEGCompressor::~JPEGCompressor() 
{
    std::cerr << "~JPEGCompressor" << std::endl;
    free(compression_buffer);
}

void JPEGCompressor::receive(core::MessagePtr m) {

    // Get the image (zero-copy)
    xt::xtensor<uint8_t, 3> rgb_tensor = serialization::deserialize_flex_tensor<uint8_t, 3>(m->root_map()[this->image_key], false);

    int height = rgb_tensor.shape()[0];
    int width = rgb_tensor.shape()[1];
    int num_channels = 3;

    const uint8_t *pImage_data = rgb_tensor.data();

    // EITHER write to file or write to and signal a message
    if (filename_prefix.size() > 0) {

        string filename = filename_prefix + std::to_string(m->timestamp()) + ".jpg";

        // Writes JPEG image to a file. 
        // num_channels must be 1 (Y) or 3 (RGB), image pitch must be width*num_channels.
        bool succeeded = jpge::compress_image_to_jpeg_file(
            filename.c_str(), 
            width, 
            height, 
            num_channels, 
            pImage_data
            //const params &comp_params = params()
        );

        if (!succeeded) {
            std::cerr << "JPEGCompressor::receive failed to call compress_image_to_jpeg_file, height=" << height << " width=" << width << std::endl;
        } else {
            if (debug) {
                std::cerr << "JPEGCompressor::receive wrote to file \"" << filename << "\", height=" << height << " width=" << width << std::endl;
            }
        }

    } else {

        int input_tensor_size = width * height * 3;
        if (compression_buffer_size < input_tensor_size) {
            compression_buffer_size = input_tensor_size;
            free(compression_buffer);
            compression_buffer = malloc(compression_buffer_size);
        }

        int buf_size = compression_buffer_size;

        // Writes JPEG image to memory buffer. 
        // On entry, buf_size is the size of the output buffer pointed at by pBuf, which should be at least ~1024 bytes. 
        // If return value is true, buf_size will be set to the size of the compressed data.
        bool succeeded = jpge::compress_image_to_jpeg_file_in_memory(
            compression_buffer, 
            buf_size, 
            width, 
            height, 
            num_channels, 
            pImage_data
            //const params &comp_params = params()
        );

        if (!succeeded) {
            std::cerr << "JPEGCompressor::receive failed to call compress_image_to_jpeg_file_in_memory, height=" << height << " width=" << width << std::endl;
        } else {
            auto o = std::make_shared<JPEGImage>(compression_buffer, buf_size, this->output_key, *m, std::set{this->image_key});
            
            // Or, we can do it without a sub-class
            // auto o = std::make_shared<core::Message>("util_jpeg", "JPEGImage", *m, std::set{this->image_key}, [&](flexbuffers::Builder& fbb) {
            //     fbb.Key(this->output_key);
            //     fbb.Blob(compression_buffer, buf_size);
            // });

            if (debug) {
                int total_bytes = height * width * 3;
                std::cerr << "JPEGCompressor::receive wrote to memory: " << buf_size << " bytes" << 
                    ", from height=" << height << 
                    ", width=" << width << 
                    ", total bytes=" << total_bytes << 
                    ", compressed to=" << std::fixed << std::setprecision(3) << ((float)buf_size / total_bytes * 100.0) << "\%" << 
                    " -> signalling " << o->to_string() << std::endl;
            }
            
            this->signal(o);
        }
    }
}


// -- JPEGDecompressor --

JPEGDecompressor::JPEGDecompressor(const string& input_key, const string& output_key, const string& name, bool debug):
    core::Node(name),
    input_key(input_key),
    output_key(output_key),
    debug(debug)
{

}

void JPEGDecompressor::receive(core::MessagePtr m) {

    auto jpeg_blob = m->root_map()[this->input_key].AsBlob();
    const uint8_t* jpeg_raw_data = jpeg_blob.data();
    int jpeg_raw_data_size = jpeg_blob.size();

    int width = 0;
    int height = 0;
    int actual_comps = 0; 
    int req_comps = 3; // req_comps can be 1 (grayscale), 3 (RGB), or 4 (RGBA). 
 
    unsigned char *rgb_data = jpgd::decompress_jpeg_image_from_memory(
        jpeg_raw_data, 
        jpeg_raw_data_size, 
        &width, 
        &height, 
        &actual_comps, 
        req_comps);

    if (rgb_data == nullptr) {

        std::cerr << "JPEGDecompressor::receive FAILED to call decompress_jpeg_image_from_memory" << std::endl;
    
    } else {

        std::vector<size_t> vshape = std::vector<size_t>{(size_t)height, (size_t)width, 3};
        size_t total_bytes = width * height * 3;
        auto tensor = xt::adapt(rgb_data, total_bytes, xt::no_ownership(), vshape);
        
        // Get a copy of the original message, omit the jpeg key/value, and insert the rgb
        auto o = std::make_shared<core::Message>("util_jpeg", "RGBImage", *m, std::set{this->input_key}, [&](flexbuffers::Builder& fbb) {
            serialization::serialize_flex_tensor<uint8_t, 3>(fbb, tensor, this->output_key);
        });

        if (debug) {
            std::cerr << "JPEGDecompressor::receive decompressed into height=" << height << ", width=" << width << " -> " << o->to_string() << std::endl;
        }

        this->signal(o);
    }
}

} // namespace utiljpeg
} // namespace roboflex
