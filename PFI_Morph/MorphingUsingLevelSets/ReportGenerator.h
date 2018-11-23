#ifndef HTML_REPORT_GENERATOR_H
#define HTML_REPORT_GENERATOR_H
#include <iostream>

/**
 * An easier to use class for generating reports.
 */
class ReportGenerator {
    std::string fname;
public:
    ReportGenerator(std::string fname): fname(fname) {}

    /**
     * Write the report to disk
     */
    void write_report();

    /**
     * Write everything up to the table of stats
     */
    void write_header(std::ostream& stream);

    /**
     * Finish everything after the table of stats
     */
    void write_footer(std::ostream& stream);
};

#endif
