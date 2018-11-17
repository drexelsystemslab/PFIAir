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
    time_point<system_clock> start;
    /**
     * End of interval
     */
    time_point<system_clock> end;
    /**
     * Message to be displayed
     */
    std::string message;
public:
    /**
     * Create a timer with a message.
     * This method starts the timer and prints
     * "Start <msg>" to the console if the flag is set
     */
    Timer(std::string msg, bool show_message=true);

    /**
     * Stop the timer and print "<msg> took <n> seconds" to the console
     * if the flag is set
     */
    double stop(bool show_message=true);
};
#endif
