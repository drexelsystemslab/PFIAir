#include "ReportGenerator.h"
#include <fstream>

void ReportGenerator::write_report() {
    std::ofstream file(fname);
    if (!file)
        throw new std::runtime_error("Cannot open " + fname);

    write_header(file);

    // TODO: Write table

    write_footer(file);

    file.close();
}

void ReportGenerator::write_header(std::ostream& stream) {
    stream 
        << "<html>" << std::endl
        << "<head>" << std::endl
        << "    <link rel='stylesheet' href='style.css'>" << std::endl
        << "    <link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/css/bootstrap.min.css' integrity='sha384-/Y6pD6FV/Vv2HJnA6t+vslU6fwYXjCFtcEpHbNJ0lyAFsXTsjBbfaDjzALeQsN6M' crossorigin='anonymous'>" << std::endl
        << "</head>" << std::endl
        << "<body>" << std::endl;
        /*
        << "    <img src='images/" + main_obj + ".png' />" << std::endl
        */
}

void ReportGenerator::write_footer(std::ostream& stream) {
        stream << "    <script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>" << std::endl
        << "    <script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.11.0/umd/popper.min.js' integrity='sha384-b/U6ypiBEHpOf/4+1nzFpr53nxSS+GLCkfwBdFNTxtclqqenISfwAzpKaMNFNmj4' crossorigin='anonymous'></script>" << std::endl
        << "    <script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/js/bootstrap.min.js' integrity='sha384-h0AbiXch4ZDo7tp9hKZ4TsHbi047NrKGLO3SEJAg45jXxnGIfYzk4Si90RDIqNm1' crossorigin='anonymous'></script>" << std::endl
        << "    <script src='script.js'></script>" << std::endl
        << "</body>" << std::endl
        << "</html>" << std::endl;
}
