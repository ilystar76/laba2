#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace xo {
using Board = std::array<char, 9>;
enum class Method { Minimax, AlphaBeta };
struct Result {
    int move = -1;
    int score = 0;
    std::uint64_t nodes = 0;
    std::uint64_t cuts = 0;
    std::uint64_t hits = 0;
};
char winner(const Board& board);
Board parsePosition(std::string_view text);
Result solve(Board board, Method method);
}
