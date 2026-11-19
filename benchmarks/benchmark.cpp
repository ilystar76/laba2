#include "benchmark.hpp"
#include "sorted_scores.hpp"
#include "tree.hpp"
#include <algorithm>
#include <chrono>
#include <ostream>
#include <random>
#include <vector>

using namespace std;

Measurement measure(string board, Method method, int repetitions) {
    SearchResult result;
    auto start = chrono::steady_clock::now();
    for (int i = 0; i < repetitions; ++i) result = findMove(board, method);
    chrono::duration<double, micro> elapsed = chrono::steady_clock::now() - start;
    return {result, elapsed.count() / repetitions, repetitions};
}

void writeSearchHeader(ostream& output) {
    output << "position,empty_cells,method,row,column,score,nodes,cuts,cache_hits,repetitions,mean_microseconds\n";
}

void writeSearchRow(ostream& output, string board, Method method, Measurement measurement) {
    auto result = measurement.result;
    output << board << ',' << count(board.begin(), board.end(), '.') << ',' << methodId(method)
           << ',' << result.move / 3 + 1 << ',' << result.move % 3 + 1 << ',' << result.score
           << ',' << result.nodes << ',' << result.cuts << ',' << result.hits
           << ',' << measurement.repetitions << ',' << measurement.microseconds << '\n';
}

namespace {
template<class Dictionary>
double measureBuild(vector<SavedScore>& entries, int repetitions) {
    chrono::duration<double, micro> elapsed{};
    for (int i = 0; i < repetitions; ++i) {
        Dictionary dictionary;
        auto start = chrono::steady_clock::now();
        for (auto entry : entries) dictionary.insert(entry);
        elapsed += chrono::steady_clock::now() - start;
    }
    return elapsed.count() / repetitions;
}

void writeBuildRows(ostream& output, string order, vector<SavedScore>& entries) {
    int repetitions = 50;
    output << order << ",avl," << entries.size() << ',' << repetitions << ','
           << measureBuild<Tree<SavedScore>>(entries, repetitions) << '\n';
    output << order << ",sorted," << entries.size() << ',' << repetitions << ','
           << measureBuild<SortedScores>(entries, repetitions) << '\n';
}
}

void writeBuildComparison(ostream& output) {
    vector<SavedScore> entries;
    for (int code = 0; code < positionCount; ++code) {
        string board = positionFromCode(code);
        if (!positionError(board).empty()) continue;
        entries.push_back({2 * code + 1, findMove(board, Method::AlphaBeta).score});
    }

    output << "order,structure,positions,repetitions,mean_microseconds\n";
    writeBuildRows(output, "ascending", entries);
    reverse(entries.begin(), entries.end());
    writeBuildRows(output, "descending", entries);
    mt19937 random(42);
    shuffle(entries.begin(), entries.end(), random);
    writeBuildRows(output, "shuffled", entries);
}
