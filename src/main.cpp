#include "game.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: solver minimax|alphabeta|compare POSITION\n";
        return 2;
    }
    try {
        const auto board = xo::parsePosition(argv[2]);
        const std::string_view mode = argv[1];
        if (mode != "minimax" && mode != "alphabeta" && mode != "compare") {
            throw std::invalid_argument("invalid_method");
        }
        const auto print = [&](const char* name, xo::Method method, int repetitions) {
            using Clock = std::chrono::steady_clock;
            const auto start = Clock::now();
            xo::Result result;
            for (int i = 0; i < repetitions; ++i) result = xo::solve(board, method);
            const auto elapsed = std::chrono::duration<double, std::micro>(Clock::now() - start);
            std::cout << "{\"method\":\"" << name << "\",\"move\":" << result.move
                      << ",\"score\":" << result.score << ",\"nodes\":" << result.nodes
                      << ",\"cuts\":" << result.cuts << ",\"hits\":" << result.hits
                      << ",\"repetitions\":" << repetitions
                      << ",\"microseconds\":" << elapsed.count() / repetitions << '}';
        };
        std::cout << '[';
        if (mode != "alphabeta") print("minimax", xo::Method::Minimax, mode == "compare" ? 100 : 1);
        if (mode == "compare") std::cout << ',';
        if (mode != "minimax") print("alphabeta", xo::Method::AlphaBeta, mode == "compare" ? 100 : 1);
        std::cout << "]\n";
    } catch (const std::invalid_argument& error) {
        std::cout << "{\"error\":\"" << error.what() << "\"}\n";
        return 2;
    }
}
