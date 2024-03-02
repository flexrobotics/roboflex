#include <algorithm>
#include <sys/time.h>
#include <cmath>
#include <thread>
#include <sstream>
#include "roboflex_core/util/utils.h"

namespace roboflex::core {

std::string string_from_fixed(const char* s, int len) 
{
    std::string r;
    r.assign(s, std::find(s, s+len, '\0'));
    return r;
}

std::string repeated_string(const std::string& input, size_t num)
{
    std::ostringstream os;
    std::fill_n(std::ostream_iterator<std::string>(os), num, input);
    return os.str();
}

std::string get_roboflex_core_version() 
{
    return std::string("0.1.33");
}

double get_current_time() 
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (double)tv.tv_sec + ((double)tv.tv_usec) / 1000000.0;
}

void sleep_ms(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void sleep_until_next_interval(
    const std::chrono::time_point<std::chrono::steady_clock> & start_t,
    double interval)
{
    const std::chrono::time_point<std::chrono::steady_clock> t =
        std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = t - start_t;
    double dtc = dt.count();
    double dt_rem = std::fmod(dtc, interval);
    auto c_dt_rem = std::chrono::duration<double>(dt_rem);
    auto c_interval = std::chrono::duration<double>(interval);
    auto next_t = t - c_dt_rem + c_interval;
    std::this_thread::sleep_until(next_t);
}

} // namespace roboflex::core
