#ifndef ROBOFLEX_CORE_UTILS__H
#define ROBOFLEX_CORE_UTILS__H

#include <string>
#include <chrono>

namespace roboflex::core {

/**
 * Constructs an std::string from an optionally null-terminated
 * fixed-length character array.
 */
std::string string_from_fixed(const char* s, int len);

/**
 * Repeats the given string the given number of times.
 */
std::string repeated_string(const std::string& input, size_t num);

/**
 * Gets the current roboflex version.
 */
std::string get_roboflex_core_version();

/**
 * Gets the current system time down to microseconds.
 */
double get_current_time();

/**
 * Sleeps the thread for the given number of milliseconds.
 */
void sleep_ms(int ms);

/**
 * Sleeps the thread until the next interval 
 */
void sleep_until_next_interval(
    const std::chrono::time_point<std::chrono::steady_clock> & start_t,
    double interval);

} // namespace roboflex::core

#endif // ROBOFLEX_CORE_UTILS__H
