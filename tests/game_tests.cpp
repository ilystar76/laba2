#include "game.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace xo;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

int main() {
    require(winner(Board{'X','.','.','.','X','.','.','.','X'}) == 'X', "diagonal");
    require(winner(Board{'.','O','.','.','O','.','.','O','.'}) == 'O', "column");
    require(winner(Board{'X','X','X','.','O','.','.','.','O'}) == 'X', "row");
    require(solve(parsePosition("OO.XX.X.."), Method::AlphaBeta).move == 2, "win");
    require(solve(parsePosition("XX..O...."), Method::Minimax).move == 2, "block");
    require(solve(parsePosition("XOX.OXOX."), Method::AlphaBeta).score == 0, "draw");

    for (const auto invalid : {"X........X", "?........", ".........", "XXXOO...."}) {
        bool rejected = false;
        try { parsePosition(invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, invalid);
    }

    int checked = 0;
    for (int key = 0; key < 19683; ++key) {
        std::string position;
        int digits = key;
        for (int i = 0; i < 9; ++i, digits /= 3) position += ".XO"[digits % 3];
        Board board;
        try { board = parsePosition(position); }
        catch (const std::invalid_argument&) { continue; }

        const auto full = solve(board, Method::Minimax);
        const auto pruned = solve(board, Method::AlphaBeta);
        require(full.move >= 0 && full.move < 9 && board[full.move] == '.', "legal move");
        require(full.move == pruned.move && full.score == pruned.score, position.c_str());
        require(pruned.nodes <= full.nodes, "expected pruning");
        ++checked;
    }
    require(checked == 2097, "position count");
    std::cout << "Compared " << checked << " nonterminal O-turn positions\n";
}
