#ifndef ROBOFLEX_WEBCAM__H
#define ROBOFLEX_WEBCAM__H

#include <vector>
#include <string_view>
#include <iostream>
#include <xtensor/xtensor.hpp>
#include <libuvc/libuvc.h>
#include "core/node.h"
#include "core/serialization/flex_xtensor.h"

using std::string, std::vector, std::exception, std::ostream;

namespace roboflex {
namespace webcam_uvc {

constexpr char ModuleName[] = "webcam_uvc";


/*
 * Wrapper class for the uvc_device_descriptor.
 * Includes identifying information about device.
 */
struct DeviceDescriptor {
    DeviceDescriptor(uvc_device_descriptor* native_device_descriptor);

    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdUVC;
    string serialNumber;
    string manufacturer;
    string product;

    string to_string() const;
};


/*
 * Function to get list of DeviceDescriptors
 */
vector<DeviceDescriptor> get_device_list();
string get_device_list_string();


/**
 * The datatype containing webcam frames.
 */
struct WebcamDataRaw : public core::Message {

    inline static const char MessageName[] = "WebcamDataRaw";

    WebcamDataRaw(core::Message& other): core::Message(other) {}
    WebcamDataRaw(uvc_frame_t* frame, double t0, double t1);

    double get_t0() const {
        return root_map()["t0"].AsDouble();
    }

    double get_t1() const {
        return root_map()["t1"].AsDouble();
    }

    double get_capture_time() const {
        return root_map()["t"].AsDouble();
    }

    uvc_frame_format get_uvc_frame_format() const {
        auto r = root_map()["f"].AsUInt32();
        return uvc_frame_format(r);
    }

    uint32_t get_width() const {
        return root_map()["w"].AsUInt32();
    }

    uint32_t get_height() const {
        return root_map()["h"].AsUInt32();
    }

    uint32_t get_sequence() const {
        return root_map()["s"].AsUInt32();
    }

    const uint8_t* get_data() const {
        auto data_portion = root_map()["data"].AsBlob();
        const uint8_t* const_data = data_portion.data();
        return const_data;
    }

    size_t get_data_size_bytes() const {
        auto data_portion = root_map()["data"].AsBlob();
        return data_portion.size();
    }

    void print_on(ostream& os) const override;
};

/**
 * The datatype containing webcam frames.
 */
struct WebcamDataRGB : public core::Message {
    
    typedef xt::xtensor<uint8_t, 3> WebcamFrame;

    inline static const char MessageName[] = "WebcamDataRGB";

    WebcamDataRGB(core::Message& other): Message(other) {}
    WebcamDataRGB(uvc_frame_t* frame, double t0, double t1);

    double get_t0() const {
        return root_map()["t0"].AsDouble();
    }

    double get_t1() const {
        return root_map()["t1"].AsDouble();
    }

    double get_capture_time() const {
        return root_map()["t"].AsDouble();
    }

    uint32_t get_sequence() const {
        return root_map()["s"].AsUInt32();
    }

    WebcamFrame get_rgb() const {
        return serialization::deserialize_flex_tensor<uint8_t, 3>(root_map()["rgb"]);
    }

    void print_on(ostream& os) const override;
};


/**
 * Can be thrown by WebcamSensor
 */
struct WebcamException : exception {
    string reason;
    WebcamException(const string& reason) : std::exception(), reason(reason) {}
    const char* what() const noexcept {
        return reason.c_str();
    }
};


/**
 * Reads from webcam
 */
class WebcamSensor : public core::RunnableNode {
public:
    WebcamSensor(
        uint16_t width,
        uint16_t height,
        uint16_t fps,
        int8_t device_index = -1,
        uvc_frame_format format = UVC_FRAME_FORMAT_ANY,
        bool emit_rgb = true,
        const string& name = "webcam_sensor");
    virtual ~WebcamSensor();
    void print_device_info();

protected:
    void child_thread_fn();
    uint16_t width;
    uint16_t height;
    uint16_t fps;
    bool emit_rgb;
    uvc_context_t* context;
    uvc_device_t* device;
    uvc_device_handle_t* device_handle = nullptr;
    uvc_stream_ctrl_t stream_ctrl;
    uvc_device_t** list;
};

class WebcamRawToRGBConverter: public core::Node {
public:
    WebcamRawToRGBConverter(
        const string& name = "webcam_raw_to_rgb"):
            core::Node(name) {}

    void receive(core::MessagePtr m) override;
};

}  // namespace webcam_uvc
}  // namespace roboflex

#endif  // ROBOFLEX_WEBCAM__H
