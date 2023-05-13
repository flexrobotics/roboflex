#ifndef ROBOFLEX_VISUALIZATION__H
#define ROBOFLEX_VISUALIZATION__H

#include <mutex>
#include <SDL2/SDL.h>
#include "core/core.h"
#include "core/core_nodes/frequency_generator.h"

namespace roboflex {
namespace visualization {

using std::string, std::pair, std::mutex, core::MessagePtr;

class RGBImageTV: public nodes::FrequencyGenerator {
public:
    RGBImageTV(
        const float frequency_hz,
        const size_t width = 640,
        const size_t height = 480,
        const string& image_key = "rgb",
        const pair<int, int>& initial_pos = {-1, -1},
        const bool mirror = false,
        const bool debug = false,
        const string& name = "RGBImageTV");
    
    virtual ~RGBImageTV();

    void receive(MessagePtr m) override;
 
    string to_string() const override;

protected:

    // overridden from FrequencyGenerator
    void on_trigger(double t) override;

    void draw_from_last_message();

    size_t width = 640;
    size_t height = 480;
    string image_key;
    bool mirror = false;
    bool debug = false;

    SDL_Window * window;

    mutable mutex last_msg_mutex;
    MessagePtr last_msg;
};

}  // namespace visualization
}  // namespace roboflex

#endif // ROBOFLEX_VISUALIZATION__H

