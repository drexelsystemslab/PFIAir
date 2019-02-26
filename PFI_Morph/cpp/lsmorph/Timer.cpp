#include "Timer.h"
#include <iostream>


void Timer::start(bool show_message) {
    if (show_message)
        std::cout << "╔═ Start " << message << std::endl;
    start_time = system_clock::now();
}

double Timer::stop(bool show_message) {
    end_time = system_clock::now();
    duration<double> elapsed_seconds = end_time - start_time;

    double secs = elapsed_seconds.count();

    if (show_message)
        std::cout << "╚═ " << message << " took " <<  secs << " seconds." 
            << std::endl;
    
    return secs;
}
