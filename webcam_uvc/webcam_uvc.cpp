#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <xtensor/xadapt.hpp>
#include <xtensor/xio.hpp>
#include "webcam_uvc/webcam_uvc.h"
#include "core/util/utils.h"
#include "core/serialization/flex_eigen.h"

namespace roboflex {
namespace webcam_uvc {


string uvc_frame_format_to_string(uvc_frame_format f)
{
    // from https://github.com/libuvc/libuvc/blob/master/include/libuvc/libuvc.h
    string ff;
    switch (f) {
        //case uvc_frame_format::UVC_FRAME_FORMAT_UNKNOWN:      ff = "UVC_FRAME_FORMAT_UNKNOWN"; break;
        /** Any supported format */
        case uvc_frame_format::UVC_FRAME_FORMAT_ANY:          ff = "UVC_FRAME_FORMAT_ANY"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_UNCOMPRESSED: ff = "UVC_FRAME_FORMAT_UNCOMPRESSED"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_COMPRESSED:   ff = "UVC_FRAME_FORMAT_COMPRESSED"; break;
        /** YUYV/YUV2/YUV422: YUV encoding with one luminance value per pixel and
         * one UV (chrominance) pair for every two pixels::
         */
        case uvc_frame_format::UVC_FRAME_FORMAT_YUYV:         ff = "UVC_FRAME_FORMAT_YUYV"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_UYVY:         ff = "UVC_FRAME_FORMAT_UYVY"; break;
        /** 24-bit RGB */
        case uvc_frame_format::UVC_FRAME_FORMAT_RGB:          ff = "UVC_FRAME_FORMAT_RGB"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_BGR:          ff = "UVC_FRAME_FORMAT_BGR"; break;
        /** Motion-JPEG (or JPEG) encoded images */
        case uvc_frame_format::UVC_FRAME_FORMAT_MJPEG:        ff = "UVC_FRAME_FORMAT_MJPEG"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_H264:         ff = "UVC_FRAME_FORMAT_H264"; break;
        /** Greyscale images */
        case uvc_frame_format::UVC_FRAME_FORMAT_GRAY8:        ff = "UVC_FRAME_FORMAT_GRAY8"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_GRAY16:       ff = "UVC_FRAME_FORMAT_GRAY16"; break;
        /* Raw colour mosaic images */
        case uvc_frame_format::UVC_FRAME_FORMAT_BY8:          ff = "UVC_FRAME_FORMAT_BY8"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_BA81:         ff = "UVC_FRAME_FORMAT_BA81"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_SGRBG8:       ff = "UVC_FRAME_FORMAT_SGRBG8"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_SGBRG8:       ff = "UVC_FRAME_FORMAT_SGBRG8"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_SRGGB8:       ff = "UVC_FRAME_FORMAT_SRGGB8"; break;
        case uvc_frame_format::UVC_FRAME_FORMAT_SBGGR8:       ff = "UVC_FRAME_FORMAT_SBGGR8"; break;
        /** YUV420: NV12 */
        case uvc_frame_format::UVC_FRAME_FORMAT_NV12:         ff = "UVC_FRAME_FORMAT_NV12"; break;
        /** YUV: P010 */
        //case uvc_frame_format::UVC_FRAME_FORMAT_P010:         ff = "UVC_FRAME_FORMAT_P010"; break;
        /** Number of formats understood */
        //UVC_FRAME_FORMAT_COUNT,
        default:                                              ff = "unknown"; break;
    }
    return ff;
}

string uvc_frame_to_string(uvc_frame_t* f) 
{
    std::stringstream sst;
    sst << "<uvc_frame" 
        << " data: " << f->data
        << " bytes: " << f->data_bytes
        << " width: " << f->width
        << " height: " << f->height
        << " frame_format: " << f->frame_format << " " << uvc_frame_format_to_string(f->frame_format)
        << " step: " << f->step
        << " sequence: " << f->sequence
        << " metadata_bytes: " << f->metadata_bytes
        << ">";
    return sst.str();
}

vector<DeviceDescriptor> get_device_list()
{
    uvc_error_t err;
    uvc_context_t* ctx;
    uvc_device_t** list;
    uvc_device_t* dev;
    uvc_device_descriptor_t* desc;
    vector<DeviceDescriptor> devices;

    err = uvc_init(&ctx, NULL);
    if (err < 0) {
        throw WebcamException(string("cannot initialize uvc context:") + std::to_string(err));
    }

    err = uvc_get_device_list(ctx, &list);
    if (err < 0) {
        throw WebcamException(string("cannot get device list:") + std::to_string(err));
    }

    int i = 0;
    while (list[i] != NULL) {
        dev = list[i];

        err = uvc_get_device_descriptor(dev, &desc);
        if (err < 0) {
            throw WebcamException(string("cannot get device descriptor:") + std::to_string(err));
        }

        devices.emplace(devices.end(), desc);

        i++;
    }

    uvc_free_device_list(list, 0);

    return devices;
}

string get_device_list_string()
{
    std::stringstream sst;
    auto devices = get_device_list();
    for (auto d: devices) {
        sst << d.to_string() << std::endl;
    }
    return sst.str();
}


// --- Device Descriptor ---

DeviceDescriptor::DeviceDescriptor(uvc_device_descriptor* native_device_descriptor):
    idVendor(native_device_descriptor->idVendor),
    idProduct(native_device_descriptor->idProduct),
    bcdUVC(native_device_descriptor->bcdUVC),
    serialNumber(native_device_descriptor->serialNumber ? native_device_descriptor->serialNumber : "UNKNOWN"),
    manufacturer(native_device_descriptor->manufacturer ? native_device_descriptor->manufacturer : "UNKNOWN"),
    product(native_device_descriptor->product ? native_device_descriptor->product : "UNKNOWN")
{

}

string DeviceDescriptor::to_string() const
{
    std::stringstream sst;
    sst << "<DeviceDescriptor"
        << " idVendor: " << this->idVendor
        << " idProduct: " << this->idProduct
        << " bcdUVC: " << this->bcdUVC
        << " serialNumber: " << this->serialNumber
        << " manufacturer: " << this->manufacturer
        << " product: " << this->product
        << ">";
    return sst.str();
}


// --- WebcamDataRaw ---

WebcamDataRaw::WebcamDataRaw(uvc_frame_t* frame, double t0, double t1):
    Message(ModuleName, MessageName)
{
    struct timeval tv = frame->capture_time;
    double capture_time = (double)tv.tv_sec + ((double)tv.tv_usec) / 1000000.0;

    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {
        fbb.Double("t0", t0);
        fbb.Double("t1", t1);
        fbb.Double("t", capture_time);
        fbb.UInt("f", frame->frame_format);
        fbb.UInt("s", frame->sequence);
        fbb.UInt("w", frame->width);
        fbb.UInt("h", frame->height);
        fbb.Key("data");
        fbb.Blob(frame->data, frame->data_bytes);
    });
}

void WebcamDataRaw::print_on(ostream& os) const
{
    os << "<WebcamDataRaw"
       << "  t0: " << std::fixed << std::setprecision(3) << get_t0() << "  t1: " << get_t1()
       << "  sequence: " << get_sequence()
       << "  width: " << get_width()
       << "  height: " << get_height()
       << "  frame_format: " << get_uvc_frame_format()
       << "  bytes: " << get_data_size_bytes()
       << "  data: " << static_cast<const void*>(get_data())
       << "  capture_time: " << get_capture_time() << " ";
    Message::print_on(os);
    os << ">";
}


// --- WebcamDataRGB ---

WebcamDataRGB::WebcamDataRGB(uvc_frame_t* frame, double t0, double t1):
    Message(ModuleName, MessageName)
{
    uint32_t sequence = frame->sequence;
    struct timeval tv = frame->capture_time;
    double capture_time = (double)tv.tv_sec + ((double)tv.tv_usec) / 1000000.0;

    auto rgb_tensor = xt::adapt(
        static_cast<const uint8_t*>(frame->data),
        frame->data_bytes,
        xt::no_ownership(),
        xt::shape({frame->height, frame->width, (uint32_t)3}));

    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {
        fbb.Double("t0", t0);
        fbb.Double("t1", t1);
        fbb.Double("t", capture_time);
        fbb.UInt("s", sequence);
        serialization::serialize_flex_tensor<uint8_t, 3>(fbb, rgb_tensor, "rgb");
    });
}

void WebcamDataRGB::print_on(ostream& os) const
{
    os << "<WebcamDataRGB"
       << " t0: " << std::fixed << std::setprecision(3) << get_t0() << " t1: " << get_t1()
       << " rgb shape: " << xt::adapt(get_rgb().shape())
       << " sequence: " << get_sequence()
       << " capture_time: " << get_capture_time() << " ";
    Message::print_on(os);
    os << ">";
}


// --- WebcamSensor ---

WebcamSensor::WebcamSensor(
    uint16_t width,
    uint16_t height,
    uint16_t fps,
    int8_t device_index,
    uvc_frame_format format,
    bool emit_rgb,
    const string& name):
        core::RunnableNode(name),
        width(width),
        height(height),
        fps(fps),
        emit_rgb(emit_rgb)
{
    uvc_error_t err;

    err = uvc_init(&context, NULL);
    if (err < 0) {
        throw WebcamException(string("cannot initialize uvc context:") + std::to_string(err));
    }

    err = uvc_get_device_list(context, &list);
    if (err < 0) {
        throw WebcamException(string("cannot get device list:") + std::to_string(err));
    }

    if (device_index < 0) {
        err = uvc_find_device(context, &device, 0, 0, NULL);
        if (err < 0) {
            throw WebcamException(string("cannot find video device:") + std::to_string(err));
        }
    } else {
        device = list[device_index];
    }

    err = uvc_open(device, &device_handle);
    if (err < 0) {
        uvc_perror(err, "uvc_open");
        throw WebcamException(string("cannot open video device:") + std::to_string(err));
    }

    err = uvc_get_stream_ctrl_format_size(
        device_handle,
        &stream_ctrl, /* result stored in stream_ctrl */
        format,
        width,
        height,
        fps);
    if (err < 0) {
        throw WebcamException(string("cannot negotiate a stream as specified:") + std::to_string(err));
    }
}

WebcamSensor::~WebcamSensor()
{
    uvc_close(device_handle);
    uvc_unref_device(device);
    uvc_exit(context);
}

void WebcamSensor::print_device_info()
{
    std::cerr << "WebcamSensor " << this->get_name() << " device_handle:" << std::endl;
    uvc_print_diag(device_handle, stderr);
    std::cerr << "WebcamSensor stream_ctrl:" << std::endl;
    uvc_print_stream_ctrl(&stream_ctrl, stderr);
    std::cerr << "/WebcamSensor" << std::endl;
}

void WebcamSensor::child_thread_fn()
{
    uvc_frame_t* frame;
    uvc_frame_t* rgb_frame;
    if (this->emit_rgb) {
        rgb_frame = uvc_allocate_frame(width * height * 3);
    }
    uvc_stream_handle_t* stream_handle;
    uvc_error_t err;

    err = uvc_stream_open_ctrl(device_handle, &stream_handle, &stream_ctrl);
    if (err < 0) {
        throw WebcamException(string("cannot open new stream:") + std::to_string(err));
    }

    err = uvc_stream_start(stream_handle, NULL, NULL, 0);
    if (err < 0) {
        throw WebcamException(string("cannot start stream:") + std::to_string(err));
    }

    while (!this->stop_requested()) {
        double t0 = core::get_current_time();
        err       = uvc_stream_get_frame(stream_handle, &frame, 0);
        double t1 = core::get_current_time();

        if (err < 0) {
            throw WebcamException(string("cannot retrieve frame:") + std::to_string(err));
        }

        if (this->emit_rgb) {
            err = uvc_any2rgb(frame, rgb_frame);
            if (err < 0) {
                // std::cout << "USING MJPEG2RGB" << std::endl;
                err = uvc_mjpeg2rgb(frame, rgb_frame);
            }

            // std::cout << "GOT RGB FRAME: " << uvc_frame_to_string(rgb_frame) << std::endl;

            if (err < 0) {
                throw WebcamException(string("cannot convert frame to RGB:") + std::to_string(err));
            }

            this->signal(std::make_shared<WebcamDataRGB>(rgb_frame, t0, t1));
        } else {
            this->signal(std::make_shared<WebcamDataRaw>(frame, t0, t1));
        }
    }

    err = uvc_stream_stop(stream_handle);
    if (err < 0) {
        throw WebcamException(string("cannot stop stream:") + std::to_string(err));
    }

    uvc_stream_close(stream_handle);
}


// --- WebcamRawToRGBConverter ---

void WebcamRawToRGBConverter::receive(core::MessagePtr m)
{
    WebcamDataRaw r(*m);

    uvc_frame_t* frame = uvc_allocate_frame(r.get_data_size_bytes());
    frame->data = (void*)r.get_data();
    frame->data_bytes = r.get_data_size_bytes();
    frame->width = r.get_width();
    frame->height = r.get_height();
    frame->frame_format = r.get_uvc_frame_format();
    frame->library_owns_data = false;
    frame->metadata_bytes = 0;

    uvc_frame_t* rgb_frame = uvc_allocate_frame(frame->width * frame->height * 3);

    uvc_error_t err = uvc_any2rgb(frame, rgb_frame);
    if (err < 0) {
        err = uvc_mjpeg2rgb(frame, rgb_frame);
    }

    this->signal(std::make_shared<WebcamDataRGB>(rgb_frame, r.get_t0(), r.get_t1()));
}

}  // namespace webcam_uvc
}  // namespace roboflex
