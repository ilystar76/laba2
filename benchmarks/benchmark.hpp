#pragma once
#include "game.hpp"
#include <iosfwd>

struct Measurement {
    SearchResult result;
    double microseconds;
    int repetitions;
};

Measurement measure(std::string board, Method method, int repetitions);
void writeSearchHeader(std::ostream& output);
void writeSearchRow(std::ostream& output, std::string board, Method method, Measurement measurement);
void writeBuildComparison(std::ostream& output);
