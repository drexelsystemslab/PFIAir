#include "Timer.h"
#include <iostream>


Timer::Timer(std::string msg, bool show_message): message(msg) {
    if (show_message)
        std::cout << "╔═ Start " << msg << std::endl;
    start = system_clock::now();
}

double Timer::stop(bool show_message) {
    end = system_clock::now();
    duration<double> elapsed_seconds = end - start;

    double secs = elapsed_seconds.count();

    if (show_message)
        std::cout << "╚═ " << message << " took " <<  secs << " seconds." 
            << std::endl;
    
    return secs;
}
