#ifndef ROBOFLEX_EVENT__H
#define ROBOFLEX_EVENT__H

#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std::chrono_literals;

namespace roboflex {
namespace util {

// https://codereview.stackexchange.com/questions/250647/c-reimplementation-of-pythons-threading-event

class Event {
private:
    bool isSet_ = false;
    std::mutex mutex;
    std::condition_variable cv;

public:

    Event(): isSet_(false) {}

    bool isSet() {
        std::scoped_lock<std::mutex> lock(mutex);
        return isSet_;
    };

    void set() {
        std::scoped_lock<std::mutex> lock(mutex);
        isSet_ = true;
        cv.notify_all();
    };

    void clear() {
        std::scoped_lock<std::mutex> lock(mutex);
        isSet_ = false;
    };

    void wait(int timeout_milliseconds=0) {
        std::unique_lock<std::mutex> unique_lock(mutex);
        while (!isSet_) {
            if (timeout_milliseconds > 0) {
                cv.wait_for(unique_lock, timeout_milliseconds*1ms);
            } else {
                cv.wait(unique_lock);
            }
        }
    };

    bool wait_once(int timeout_milliseconds=0) {
        std::unique_lock<std::mutex> unique_lock(mutex);
        if (isSet_) {
            return true;
        } else {
            if (timeout_milliseconds > 0) {
                cv.wait_for(unique_lock, timeout_milliseconds*1ms);
            } else {
                cv.wait(unique_lock);
            }
        }
        return isSet_;
    };
};

} // namespace util
} // namespace roboflex

#endif // ROBOFLEX_EVENT__H
