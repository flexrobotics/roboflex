// https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-runtime-using-c

/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#ifndef ROBOFLEX_GET_PROCESS_MEMORY_USAGE__H
#define ROBOFLEX_GET_PROCESS_MEMORY_USAGE__H


namespace roboflex {
namespace util {

/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
size_t getPeakRSS();


/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
size_t getCurrentRSS();


} // namespace util
} // namespace roboflex

#endif // ROBOFLEX_GET_PROCESS_MEMORY_USAGE__H
