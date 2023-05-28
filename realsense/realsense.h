#ifndef ROBOFLEX_REALSENSE__H
#define ROBOFLEX_REALSENSE__H

#include <librealsense2/rs_advanced_mode.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xio.hpp>
#include "core/core.h"

namespace roboflex {
namespace realsense {

constexpr char ModuleName[] = "realsense";

using namespace std;

using RGBFrame   = xt::xtensor<uint8_t, 3>;   // [row, col, color_channel], intensity
using DepthFrame = xt::xtensor<uint16_t, 2>;  // [row, col], depth in millimeters
using CameraK    = xt::xtensor<float, 2>;     // 3x3 intrinsics matrix

enum class CameraType: uint8_t { NONE, RGB, DEPTH };

class RealsenseFrameset: public core::Message {
public:

    inline static const char MessageName[] = "RealsenseFrameset";

    RealsenseFrameset(core::Message& other): core::Message(other, MessageName) {}
    RealsenseFrameset(
        double t0,
        double t1,
        const string& serial_number,
        const rs2::frameset& rs_frameset,
        const CameraType aligned_to,
        const CameraK& camera_k_rgb,
        const CameraK& camera_k_depth);

    double get_t0() const {
        return root_map()["t0"].AsDouble();
    }

    double get_t1() const {
        return root_map()["t1"].AsDouble();
    }

    const serialization::flextensor_adaptor<uint8_t> get_rgb() const {
        return serialization::deserialize_flex_tensor<uint8_t, 3>(root_map()["rgb"]);
    }

    const serialization::flextensor_adaptor<uint16_t> get_depth() const {
        return serialization::deserialize_flex_tensor<uint16_t, 2>(root_map()["depth"]);
    }

    double get_timestamp() const {
        return root_map()["t"].AsDouble();
    }

    uint64_t get_frame_number() const {
        return root_map()["n"].AsUInt64();
    }

    string get_serial_number() const {
        return root_map()["serial_number"].AsString().str();
    }

    CameraK get_camera_k_rgb() const {
        return serialization::deserialize_flex_tensor<float, 2>(root_map()["camera_k_rgb"]);
    }

    CameraK get_camera_k_depth() const {
        return serialization::deserialize_flex_tensor<float, 2>(root_map()["camera_k_depth"]);
    }

    CameraType get_aligned_to() const {
        uint8_t at =  root_map()["aligned_to"].AsUInt8();
        return static_cast<CameraType>(at);
    }

    void print_on(ostream& os) const override;
};


struct CameraIntrinsics {
    /// in pixels
    int width;

    /// in pixels
    int height;

    /// Horizontal coordinate of the principal point of the image, as a pixel offset
    /// from the left edge.
    float ppx;

    /// Vertical coordinate of the principal point of the image, as a pixel offset from
    /// the top edge.
    float ppy;

    /// Focal length of the image plane, as a multiple of pixel width.
    float fx;

    /// Focal length of the image plane, as a multiple of pixel height.
    float fy;

    // TODO: Add in distortion model
    // rs2_distortion model; /// Distortion model of the image
    // float coeffs[5]; /// Distortion coefficients, order: k1, k2, p1, p2, k3

    CameraIntrinsics(const rs2_intrinsics& rs_intrinsics);

    CameraK to_k_matrix() const {
        return {
            { fx,  0, ppx },
            {  0, fy, ppy },
            {  0,  0,   1 },
        };
    }
};


struct RealsenseConfig
{
    /// The realsense has two sensors - RGB and depth. This controls which one the
    /// frames get aligned to. If nullopt, no alignment is performed
    CameraType align_to = CameraType::NONE;

    /// When `true`, allows fps to drop in order to better expose frames,
    /// such as in dimly lit environments
    bool prioritize_ae = false;

    struct CameraSettings {
        /// If 0, any valid value may be used.
        unsigned int fps    = 0;
        unsigned int width  = 0;
        unsigned int height = 0;
    };
    CameraSettings rgb_settings;
    CameraSettings depth_settings;

    enum class D400VisualPreset: uint8_t {
        CUSTOM = 0,
        DEFAULT = 1,
        HAND = 2,
        HIGH_ACCURACY = 3,
        HIGH_DENSITY = 4,
        MEDIUM_DENSITY = 5,
    };
    D400VisualPreset depth_visual_preset = D400VisualPreset::DEFAULT;

    struct TemporalFilterParameters {
        float alpha = 0.4;
        float delta = 20.0;
        int persistence_control = 7;
    };
    optional<TemporalFilterParameters> temporal_filter_parameters = nullopt;

    optional<int> hole_filling_mode = nullopt;

    optional<int> decimation_filter = nullopt;

    string to_string() const;
};


class RealsenseSensor: public core::RunnableNode {
public:
    RealsenseSensor(
        const string& serial_number,
        const RealsenseConfig& config,
        const string& name="RealsenseSensor");

    virtual ~RealsenseSensor();

    static set<string> get_connected_device_serial_numbers();
    static shared_ptr<RealsenseSensor> get_one_sensor(const RealsenseConfig& config, const string& name="RealsenseSensor");

    void produce();

    string get_serial_number() const { return serial_number; }
    RealsenseConfig get_config() const { return config; }
    CameraIntrinsics get_intrinsics(const CameraType& cam) const;
    CameraK get_color_camera_k() const { return camera_k_rgb; }
    CameraK get_depth_camera_k() const { return camera_k_depth; }
    CameraK get_camera_k(const CameraType& cam) const;
    int get_width_pixels_color() const { return width_pixels_color; }
    int get_height_pixels_color() const { return height_pixels_color; }
    int get_width_pixels_depth() const { return width_pixels_depth; }
    int get_height_pixels_depth() const { return height_pixels_depth; }
    int get_fps_color() const { return fps_color; }
    int get_fps_depth() const { return fps_depth; }

    string to_string() const override;

protected:
    void child_thread_fn() override;
    rs2::video_stream_profile get_video_stream_profile(const CameraType& cam) const;

    RealsenseConfig config;
    string serial_number;
    CameraK camera_k_rgb;
    CameraK camera_k_depth;
    int width_pixels_color;
    int height_pixels_color;
    int width_pixels_depth;
    int height_pixels_depth;
    int fps_color;
    int fps_depth;

    rs2::pipeline pipe;
    optional<rs2::align> aligner;
    optional<rs2::temporal_filter> temporal_filter;
    optional<rs2::hole_filling_filter> hole_filling_filter;
    optional<rs2::decimation_filter> decimation_filter;

    static rs2::context ctx;
    static set<string> extant_devices;
};

} // namespace realsense
} // namespace roboflex

#endif // ROBOFLEX_REALSENSE__H
