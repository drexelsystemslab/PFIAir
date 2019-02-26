#ifndef PROFILING_TIMER_H
#define PROFILING_TIMER_H
#include <chrono>
#include <string>
using namespace std::chrono;

/**
 * Simple profiling timer that measures
 * elapsed time and pretty prints the results
 * with a message
 */
class Timer {
    /**
     * Start of time interval
     */
    time_point<system_clock> start_time;
    /**
     * End of interval
     */
    time_point<system_clock> end_time;
    /**
     * Message to be displayed
     */
    std::string message;
public:
    /**
     * Create a timer with a message.
     */
    Timer(std::string msg): message(msg) {}

    /**
     * This method starts the timer and prints
     * "Start <msg>" to the console if the flag is set
     */
    void start(bool show_message=true);

    /**
     * Stop the timer and print "<msg> took <n> seconds" to the console
     * if the flag is set
     */
    double stop(bool show_message=true);
};
#endif
