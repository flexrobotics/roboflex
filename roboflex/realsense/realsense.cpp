#include "realsense.h"

namespace roboflex {
namespace realsense {


string align_to_to_string(const CameraType align_to)
{
    return align_to == CameraType::NONE ? "NONE" :
           align_to == CameraType::RGB ? "RGB" : 
           "DEPTH";
}

string camera_settings_to_string(const RealsenseConfig::CameraSettings& s)
{
    stringstream sst;
    sst << "<CameraSettings"
        << " fps: " << s.fps
        << " width: " << s.width
        << " height: " << s.height
        << ">";
    return sst.str();
}

string visual_preset_to_string(const RealsenseConfig::D400VisualPreset& p)
{
    switch (p) {
        case RealsenseConfig::D400VisualPreset::CUSTOM: return "CUSTOM";
        case RealsenseConfig::D400VisualPreset::DEFAULT: return "DEFAULT";
        case RealsenseConfig::D400VisualPreset::HAND: return "HAND";
        case RealsenseConfig::D400VisualPreset::HIGH_ACCURACY: return "HIGH_ACCURACY";
        case RealsenseConfig::D400VisualPreset::HIGH_DENSITY: return "HIGH_DENSITY";
        case RealsenseConfig::D400VisualPreset::MEDIUM_DENSITY: return "MEDIUM_DENSITY";
    }
    return "";
}

string temporal_filter_parameters_to_string(const optional<RealsenseConfig::TemporalFilterParameters>& p)
{
    stringstream sst;
    sst << "<TemporalFilterParameters";
    if (p.has_value()) {
        sst << " alpha: " << fixed << setprecision(3) << p->alpha
            << " delta: " << fixed << setprecision(3) << p->delta
            << " persistence_control: " << p->persistence_control;
    } else {
        sst << " empty";
    }
    sst << ">";
    return sst.str();
}

string hole_filling_mode_to_string(optional<int> mode) {
    return mode.has_value() ? ("<hole_filling_mode " + std::to_string(mode.value()) + ">") : "none";
}

rs2_stream stream_type_from_camera_type(const CameraType& cam)
{
    switch (cam) {
        case CameraType::RGB: return RS2_STREAM_COLOR;
        case CameraType::DEPTH: return RS2_STREAM_DEPTH;
        default: throw runtime_error("`CameraType` not valid!");
    }
}


// -- RealsenseFrameset --

RealsenseFrameset::RealsenseFrameset(
    double t0,
    double t1,
    const string& serial_number,
    const rs2::frameset& rs_frameset,
    const CameraType aligned_to,
    const CameraK& camera_k_rgb,
    const CameraK& camera_k_depth):
        Message(ModuleName, MessageName)
{
    rs2::video_frame rs_rgb = rs_frameset.get_color_frame();
    RGBFrame rgb = xt::adapt(
        static_cast<const RGBFrame::value_type*>(rs_rgb.get_data()),
        rs_rgb.get_data_size() / sizeof(RGBFrame::value_type),
        xt::no_ownership(),
        xt::shape({rs_rgb.get_height(), rs_rgb.get_width(), 3}));

    rs2::depth_frame rs_depth = rs_frameset.get_depth_frame();
    DepthFrame depth = xt::adapt(
        static_cast<const DepthFrame::value_type*>(rs_depth.get_data()),
        rs_depth.get_data_size() / sizeof(DepthFrame::value_type),
        xt::no_ownership(),
        xt::shape({rs_depth.get_height(), rs_depth.get_width()}));

    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {
        fbb.Double("t0", t0);
        fbb.Double("t1", t1);
        fbb.String("serial_number", serial_number);
        fbb.Double("t", rs_rgb.get_timestamp() * 0.001);
        fbb.UInt("n", (uint64_t)rs_rgb.get_frame_number());
        fbb.UInt("aligned_to", static_cast<uint8_t>(aligned_to));
        serialization::serialize_flex_tensor(fbb, rgb, "rgb");
        serialization::serialize_flex_tensor(fbb, depth, "depth");
        serialization::serialize_flex_tensor(fbb, camera_k_rgb, "camera_k_rgb");
        serialization::serialize_flex_tensor(fbb, camera_k_depth, "camera_k_depth");
    });
}

void RealsenseFrameset::print_on(ostream& os) const
{
    os << "<RealsenseFrameset"
       << " t0: " << fixed << setprecision(3) << get_t0()
       << " t1: " << fixed << setprecision(3) << get_t1()
       << " t: " << fixed << setprecision(3) << get_timestamp()
       << " serial_number: " << get_serial_number()
       << " frame_number: " << get_frame_number()
       << " aligned_to: " << align_to_to_string(get_aligned_to())
       << " rgb: (" << xt::adapt(get_rgb().shape()) << ")"
       << " depth: (" << xt::adapt(get_depth().shape()) << ")"
       << ">";
}


// -- CameraIntrinsics --

CameraIntrinsics::CameraIntrinsics(const rs2_intrinsics& intr):
    width(intr.width),
    height(intr.height),
    ppx(intr.ppx),
    ppy(intr.ppy),
    fx(intr.fx),
    fy(intr.fy)
{

}


// -- RealsenseConfig --

string RealsenseConfig::to_string() const
{
    stringstream sst;
    sst << "<RealsenseConfig"
        << " align_to: " << align_to_to_string(align_to)
        << " prioritize_ae: " << prioritize_ae
        << " rgb settings: " << camera_settings_to_string(rgb_settings)
        << " depth settings: " << camera_settings_to_string(depth_settings)
        << " depth_visual_preset: " << visual_preset_to_string(depth_visual_preset)
        << " temporal_filter_parameters: " << temporal_filter_parameters_to_string(temporal_filter_parameters)
        << " hole_filling_mode: " << hole_filling_mode_to_string(hole_filling_mode)
        << ">";
    return sst.str();
}


// -- RealsenseSensor --

rs2::context RealsenseSensor::ctx;
set<string> RealsenseSensor::extant_devices;

RealsenseSensor::RealsenseSensor(
    const string& serial_number,
    const RealsenseConfig& config,
    const string& name):
        RunnableNode(name),
        config(config),
        serial_number(serial_number)
{
    if (extant_devices.count(serial_number) > 0) {
        throw runtime_error("Device with serial number " + serial_number + " is already extant.");
    }

    auto serial_numbers = get_connected_device_serial_numbers();
    if (serial_numbers.count(serial_number) == 0) {
        throw runtime_error("Unable to find realsense device with serial number " + serial_number + ".");
    }

    rs2::config rs_cfg;
    rs_cfg.enable_device(serial_number);

    auto& depth_settings = config.depth_settings;
    auto& rgb_settings   = config.rgb_settings;

    rs_cfg.enable_stream(
        RS2_STREAM_DEPTH,
        depth_settings.width,
        depth_settings.height,
        RS2_FORMAT_Z16,
        depth_settings.fps);

    rs_cfg.enable_stream(
        RS2_STREAM_COLOR,
        rgb_settings.width,
        rgb_settings.height,
        RS2_FORMAT_RGB8,
        rgb_settings.fps);

    pipe = rs2::pipeline(ctx);

    // busy loop until we can actually talk to the device for realz. Should be quick.
    auto start_time = chrono::system_clock::now();
    while (true) {
        try {
            rs_cfg.resolve(pipe);
            break;
        } catch (const rs2::error& e) {
            if (chrono::system_clock::now() - start_time > chrono::seconds(10)) {
                throw runtime_error("Could not find a valid device to configure!");
            }
            // otherwise, continue looping
        }
    }

    try {
        rs2::pipeline_profile prof = pipe.start(rs_cfg);
        rs2::device added = prof.get_device();
        string serial = added.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        cout << "Started pipeline for device: " << serial << endl;
    } catch (rs2::error& e) {
        cerr << "Error while starting pipeline: " << e.get_failed_function() << "("
                  << e.get_failed_args() << "):\n    " << e.what() << endl;
        throw e;
    }

    rs2::pipeline_profile prof = pipe.get_active_profile();

    // Set to advanced mode if the config has a visual preset.
    // In the future, there are many many more settings in this mode,
    // such as controlling the laser power, the exposure, etc. so we
    // may need another way to trigger advanced mode.
    if (this->config.depth_visual_preset != RealsenseConfig::D400VisualPreset::DEFAULT) {
        rs400::advanced_mode am(prof.get_device());
        if (!am.is_enabled()) {
            am.toggle_advanced_mode(true);
            if (!am.is_enabled()) {
                throw runtime_error("Unable to set to advanced mode, which is required by the depth_visual_preset setting.");
            }
        }
    }

    if (config.align_to != CameraType::NONE) {
        this->aligner = rs2::align((config.align_to == CameraType::RGB) ? RS2_STREAM_COLOR : RS2_STREAM_DEPTH);
    }

    if (config.temporal_filter_parameters != nullopt) {
        auto tfp = config.temporal_filter_parameters;
        this->temporal_filter = rs2::temporal_filter(tfp->alpha, tfp->delta, tfp->persistence_control);
    }

    if (config.hole_filling_mode.has_value()) {
        this->hole_filling_filter = rs2::hole_filling_filter(config.hole_filling_mode.value());
    }

    if (config.decimation_filter.has_value()) {
        this->decimation_filter = rs2::decimation_filter(config.decimation_filter.value());
    }

    auto sensors = prof.get_device().query_sensors();

    // This is how you get the first (and only) color sensor from a device. Sometimes it just be that way.
    // https://github.com/IntelRealSense/librealsense/blob/9437bc24c0f4ce16193f1045ceea703a8151b96d/examples/sensor-control/api_how_to.h#L199
    rs2::sensor color_sensor = prof.get_device().first<rs2::color_sensor>();

    // Note, options are stateful between runs. Always be sure to set them explicitly
    // to avoid accidental state.
    color_sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);
    color_sensor.set_option(RS2_OPTION_AUTO_EXPOSURE_PRIORITY, this->config.prioritize_ae);

    // set the depth preset, which as we've said above, requires advanced mode.
    if (this->config.depth_visual_preset != RealsenseConfig::D400VisualPreset::DEFAULT) {
        auto vispresetthissux = static_cast<underlying_type<RealsenseConfig::D400VisualPreset>::type>(this->config.depth_visual_preset);
        rs2::sensor depth_sensor = prof.get_device().first<rs2::depth_sensor>();
        depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, vispresetthissux);
    }


    // Read camera k matrices
    auto color_stream = get_video_stream_profile(CameraType::RGB);
    auto depth_stream = get_video_stream_profile(CameraType::DEPTH);
    CameraIntrinsics color_intrinsics = color_stream.get_intrinsics();
    CameraIntrinsics depth_intrinsics = depth_stream.get_intrinsics();
    camera_k_rgb = color_intrinsics.to_k_matrix();
    camera_k_depth = depth_intrinsics.to_k_matrix();

    // When the user specifies '0' for some settings, it will
    // pick something arbitrary. So we need to read these values
    // to determine what they actually ended up being.
    width_pixels_color = color_intrinsics.width;
    height_pixels_color = color_intrinsics.height;
    width_pixels_depth = depth_intrinsics.width;
    height_pixels_depth = depth_intrinsics.height;
    fps_color = color_stream.fps();
    fps_depth = depth_stream.fps();

    extant_devices.insert(serial_number);
}

RealsenseSensor::~RealsenseSensor()
{
    pipe.stop();
    cout << "Stopped pipeline for device: " << this->serial_number << endl;
    extant_devices.erase(serial_number);
}

shared_ptr<RealsenseSensor> RealsenseSensor::get_one_sensor(const RealsenseConfig& config, const string& name)
{
    set<string> serial_numbers = RealsenseSensor::get_connected_device_serial_numbers();
    if (serial_numbers.size() != 1) {
        throw runtime_error("get_one_sensor called, but there are " + std::to_string(serial_numbers.size()) + " sensors.");
    }
    string one_serial_number = *(serial_numbers.begin());
    return make_shared<RealsenseSensor>(one_serial_number, config, name);
}

rs2::video_stream_profile RealsenseSensor::get_video_stream_profile(const CameraType& cam) const
{
    rs2_stream stream_type = stream_type_from_camera_type(cam);
    return this->pipe.get_active_profile().get_stream(stream_type).as<rs2::video_stream_profile>();
}

CameraK RealsenseSensor::get_camera_k(const CameraType& cam) const
{
    if (cam == CameraType::NONE) {
        throw runtime_error("Cannot get_camera_k of NONE camera type.");
    }
    return cam == CameraType::RGB ? camera_k_rgb : camera_k_depth;
}

set<string> RealsenseSensor::get_connected_device_serial_numbers()
{
    auto devices = ctx.query_devices();
    auto serial_numbers = set<string>();
    for (const auto& device : devices) {
        string serial_number = device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        serial_numbers.insert(serial_number);
    }
    return serial_numbers;
}

void RealsenseSensor::produce()
{
    // Read from the device.
    double t0 = core::get_current_time();
    rs2::frameset fs = pipe.wait_for_frames();
    double t1 = core::get_current_time();

    // Align frames if there is an aligner
    if (this->aligner) fs = this->aligner->process(fs);

    // Perform decimation if there is a decimation filter
    if (this->decimation_filter) fs = this->decimation_filter->process(fs);

    // Perform temporal alignment if there is a temporal filter
    if (this->temporal_filter) fs = this->temporal_filter->process(fs);

    // Perform hole filling if there is a hole filling filter
    if (this->hole_filling_filter) fs = this->hole_filling_filter->process(fs);

    // Get some values we need to...
    const CameraType aligned_to = config.align_to;
    const string serial_number = get_serial_number();
    const CameraK camera_k_rgb = get_color_camera_k();
    const CameraK camera_k_depth = get_depth_camera_k();

    // ... create a message
    auto msg = make_shared<RealsenseFrameset>(
        t0,
        t1,
        serial_number,
        fs,
        aligned_to,
        camera_k_rgb,
        camera_k_depth);

    // signal
    this->signal(msg);
}

void RealsenseSensor::child_thread_fn()
{
    while (!this->stop_requested()) {
        produce();
    }
}

string RealsenseSensor::to_string() const
{
    stringstream sst;
    sst << "<RealsenseSensor"
        << " serial_number: " << get_serial_number() << std::endl
        << " color: (" << get_height_pixels_color() << ", " << get_width_pixels_color() << ") AT " << get_fps_color() << " FPS, K=" << std::endl << get_color_camera_k() << ", " << std::endl
        << " depth: (" << get_height_pixels_depth() << ", " << get_width_pixels_depth() << ") AT " << get_fps_depth() << " FPS, K=" << std::endl << get_depth_camera_k() << ", " << std::endl
        << " config: " << get_config().to_string()
        << " " << Node::to_string()
        << ">";
    return sst.str();
}

} // namespace realsense
} // namespace roboflex
