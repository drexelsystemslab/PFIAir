#ifndef HTML_REPORT_GENERATOR_H
#define HTML_REPORT_GENERATOR_H
#include <iostream>
#include <vector>
#include "MorphStats.h"

// Pair of two MorphStats
typedef std::pair<MorphStats, MorphStats> StatPair;

/**
 * An easier to use class for generating reports.
 */
class ReportGenerator {
    // HTML file
    std::string fname;

    // Pairs of morph stats
    std::vector<StatPair> stat_pairs;

    /**
     * Compare rows by comparing the average energies of forwards and
     * backwards transformations
     */
    //static bool compare_rows(StatPair row1, StatPair row2);
public:
    ReportGenerator(std::string fname): fname(fname) {}

    /**
     * Add stats to the report for a single pair of models
     * forward is source -> target
     * backward is source <- target
     */
    void add_row(MorphStats forward, MorphStats backward);

    /**
     * Write the report to disk
     */
    void write_report();

    /**
     * Write everything up to the table of stats
     */
    void write_header(std::ostream& stream);

    /**
     * Write the entire table
     */
    void write_table(std::ostream& stream);

    /**
     * Write a row of the table
     */
    void write_row(std::ostream& stream, const StatPair& row);

    /**
     * Finish everything after the table of stats
     */
    void write_footer(std::ostream& stream);
};


#endif
