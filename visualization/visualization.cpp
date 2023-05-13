#include "visualization.h"

namespace roboflex {
namespace visualization {

// Uses flextensor_adaptor for 0-copy
template <typename T, size_t D>
serialization::flextensor_adaptor<T> extract_tensor(core::MessagePtr m, const std::string& key, bool print_memory_address=false)
{
    return serialization::deserialize_flex_tensor<T, D>(m->get_root_as_map()[key], print_memory_address);
}

RGBImageTV::RGBImageTV(
    const float frequency_hz,
    const size_t width,
    const size_t height,
    const string& image_key,
    const pair<int, int>& initial_pos,
    const bool mirror,
    const bool debug,
    const string& name):
        nodes::FrequencyGenerator(frequency_hz, name),
        width(width),
        height(height),
        image_key(image_key),
        mirror(mirror),
        debug(debug)
{

    // initialize SDL
    int error_code = SDL_Init(SDL_INIT_VIDEO);
    if (error_code < 0) {
        throw std::runtime_error("Unable to init SDL : " + std::to_string(error_code) + ":" + SDL_GetError());
    }

    //atexit(SDL_Quit);

    this->window = SDL_CreateWindow(this->get_name().c_str(),
        initial_pos.first, initial_pos.second, 
        width, height, SDL_WINDOW_OPENGL);
}

RGBImageTV::~RGBImageTV()
{
    SDL_DestroyWindow(this->window);
    this->window = nullptr;
}

std::string RGBImageTV::to_string() const
{
    return std::string("<RGBImageTV ") +
        " width=" + std::to_string(width) + 
        " height=" + std::to_string(height) + 
        " image_key=\"" + image_key + "\""
        " mirror=" + std::to_string(mirror) + 
        " " + FrequencyGenerator::to_string() + ">";
}

void RGBImageTV::draw_from_last_message()
{
    // lock the last message
    core::MessagePtr m;
    {
        const lock_guard<mutex> lock(last_msg_mutex);
        m = last_msg;
    }

    // If there's nothing to see, just do nothing, for now
    if (m == nullptr) {
        return;
    }

    // extract the rgb frame into an xtensor
    auto rgb_frame = extract_tensor<uint8_t, 3>(m, image_key, this->debug);

    // extract the 'raw data' pointer
    uint8_t * rgb_data = rgb_frame.data();

    if (this->mirror) {
        // We have to realize (copy) the tensor, otherwise we will modify it in-place.
        auto new_rgb_frame = xt::xtensor<uint8_t, 3>(xt::flip(rgb_frame, 1));
        rgb_data = new_rgb_frame.data();
    }

    if (this->debug) {
        std::cerr << "RGBImageTV::draw_from_last_message got data tensor at:" << static_cast<void*>(rgb_data) << " with size: " << xt::adapt(rgb_frame.shape()) << std::endl;
    }

    // Get the screen
    SDL_Surface * screen = SDL_GetWindowSurface(this->window);

    // Create an sdl surface to use our raw data
    int depth_of_surface_in_bits = 24;
    int pitch = width * 3;
    SDL_Surface * rgb_surface = SDL_CreateRGBSurfaceFrom(
        rgb_data, width, height, depth_of_surface_in_bits, pitch,
        0x0000ff, 0x00ff00, 0xff0000, 0);

    if (rgb_surface == NULL) {
        std::cerr << "SDL_CreateRGBSurfaceFrom returned NULL" << std::endl;
        return;
    }

    // Render surface to the screen
    if (SDL_BlitSurface(rgb_surface, NULL, screen, NULL) == 0) {
        SDL_UpdateWindowSurface(this->window);
    }

    // Free it every time. Prolly should RAII it.
    SDL_FreeSurface(rgb_surface);
}

void RGBImageTV::on_trigger(double t)
{
    draw_from_last_message();
}

void RGBImageTV::receive(core::MessagePtr m)
{
    {
        const lock_guard<mutex> lock(last_msg_mutex);
        last_msg = m;
    }

    if (get_frequency() == 0) {
        draw_from_last_message();
    }

    // propagate
    this->signal(m);
}

}  // namespace visualization
}  // namespace roboflex
