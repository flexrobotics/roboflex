#ifndef ROBOFLEX_VISUALIZATION__H
#define ROBOFLEX_VISUALIZATION__H

#include <mutex>
#include <SDL2/SDL.h>
#include "roboflex/core/core.h"
#include "roboflex/core/core_nodes/frequency_generator.h"

namespace roboflex {
namespace visualization {

using std::string, std::pair, std::mutex, core::MessagePtr;

class Television: public nodes::FrequencyGenerator {
public:
    Television(
        const float frequency_hz,
        const size_t width = 640,
        const size_t height = 480,
        const string& image_key = "rgb",
        const pair<int, int>& initial_pos = {-1, -1},
        const bool mirror = false,
        const bool debug = false,
        const string& name = "Television");
    
    virtual ~Television();

    void receive(MessagePtr m) override;

protected:

    // overridden from FrequencyGenerator
    void on_trigger(double t) override;
    void draw_from_last_message();

    virtual void get_rgb_image(
        MessagePtr m, 
        xt::xtensor<uint8_t, 3>& into_rgb_tensor) const = 0;

    size_t width = 640;
    size_t height = 480;
    string image_key;
    bool mirror = false;
    bool debug = false;

    SDL_Window * window;

    mutable mutex last_msg_mutex;
    MessagePtr last_msg;

    xt::xtensor<uint8_t, 3> rgb_frame;
};

class RGBImageTV: public Television {
public:
    RGBImageTV(
        const float frequency_hz,
        const size_t width = 640,
        const size_t height = 480,
        const string& image_key = "rgb",
        const pair<int, int>& initial_pos = {-1, -1},
        const bool mirror = false,
        const bool debug = false,
        const string& name = "RGBImageTV"):
            Television(frequency_hz, width, height, image_key, initial_pos, mirror, debug, name) {}
 
    string to_string() const override;

protected:

    void get_rgb_image(
        MessagePtr m, 
        xt::xtensor<uint8_t, 3>& into_rgb_tensor) const override;
};

class DepthTV: public Television {
public:
    DepthTV(
        const float frequency_hz,
        const size_t width = 640,
        const size_t height = 480,
        const string& image_key = "depth",
        const pair<int, int>& initial_pos = {-1, -1},
        const bool mirror = false,
        const bool debug = false,
        const string& name = "DepthTV"):
            Television(frequency_hz, width, height, image_key, initial_pos, mirror, debug, name) {}
 
    string to_string() const override;

protected:

    void get_rgb_image(
        MessagePtr m, 
        xt::xtensor<uint8_t, 3>& into_rgb_tensor) const override;
};


class MaskColorizer: public core::Node {
public:
    MaskColorizer(
        const string& input_key = "rgb",
        const string& output_key = "rgb",
        bool render_on_input = true,
        const string& name = "MaskColorizer"):
            core::Node(name),
            input_key(input_key),
            output_key(output_key),
            render_on_input(render_on_input) {}

    void receive(MessagePtr m) override;

protected:
    string input_key;
    string output_key;
    bool render_on_input;
};

}  // namespace visualization
}  // namespace roboflex

#endif // ROBOFLEX_VISUALIZATION__H

