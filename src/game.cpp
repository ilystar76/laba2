#include "game.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace xo {
namespace {
constexpr int lines[][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
constexpr int infinity = 20;
constexpr int moveOrder[] = {4,0,2,6,8,1,3,5,7};

enum class Bound { Exact, Lower, Upper };
struct Entry { int key; int score; Bound bound; };

class PositionTable {
public:
    const Entry* find(int key) const {
        for (const auto& entry : buckets[key % buckets.size()]) {
            if (entry.key == key) return &entry;
        }
        return nullptr;
    }

    void put(Entry value) {
        auto& bucket = buckets[value.key % buckets.size()];
        for (auto& entry : bucket) {
            if (entry.key == value.key) {
                if (entry.bound != Bound::Exact) entry = value;
                return;
            }
        }
        bucket.push_back(value);
    }

private:
    std::array<std::vector<Entry>, 257> buckets;
};

int positionKey(const Board& board, char turn) {
    int key = turn == 'O' ? 1 : 0;
    for (char cell : board) key = key * 3 + (cell == 'X' ? 1 : cell == 'O' ? 2 : 0);
    return key;
}

int search(Board& board, char turn, int alpha, int beta, Result& result, PositionTable* table) {
    ++result.nodes;
    const int empty = std::count(board.begin(), board.end(), '.');
    const char won = winner(board);
    if (won == 'O') return 1 + empty;
    if (won == 'X') return -1 - empty;
    if (!empty) return 0;

    int key = 0;
    if (table) {
        key = positionKey(board, turn);
        if (const auto* entry = table->find(key)) {
            ++result.hits;
            if (entry->bound == Bound::Exact) return entry->score;
            if (entry->bound == Bound::Lower) alpha = std::max(alpha, entry->score);
            else beta = std::min(beta, entry->score);
            if (alpha >= beta) return entry->score;
        }
    }
    const int startAlpha = alpha;
    const int startBeta = beta;
    const bool maximize = turn == 'O';
    int best = maximize ? -infinity : infinity;
    for (int index : moveOrder) {
        if (board[index] != '.') continue;
        board[index] = turn;
        const int score = search(board, maximize ? 'X' : 'O', alpha, beta, result, table);
        board[index] = '.';
        if (maximize) {
            best = std::max(best, score);
            alpha = std::max(alpha, best);
        } else {
            best = std::min(best, score);
            beta = std::min(beta, best);
        }
        if (table && alpha >= beta) {
            ++result.cuts;
            break;
        }
    }
    if (table) {
        const auto bound = best <= startAlpha ? Bound::Upper
                         : best >= startBeta ? Bound::Lower : Bound::Exact;
        table->put({key, best, bound});
    }
    return best;
}
}

char winner(const Board& board) {
    for (const auto& line : lines) {
        const char first = board[line[0]];
        if (first != '.' && first == board[line[1]] && first == board[line[2]]) return first;
    }
    return '.';
}

Board parsePosition(std::string_view text) {
    if (text.size() != 9) throw std::invalid_argument("expected_9_cells");
    Board board{};
    int x = 0;
    int o = 0;
    for (std::size_t i = 0; i < board.size(); ++i) {
        const char cell = text[i];
        if (cell != 'X' && cell != 'O' && cell != '.') throw std::invalid_argument("invalid_cell");
        x += cell == 'X';
        o += cell == 'O';
        board[i] = cell;
    }
    if (x != o + 1) throw std::invalid_argument("wrong_turn");
    if (winner(board) != '.' || x + o == 9) throw std::invalid_argument("finished");
    return board;
}

Result solve(Board board, Method method) {
    Result result;
    if (winner(board) != '.' || std::find(board.begin(), board.end(), '.') == board.end()) return result;
    PositionTable table;
    result.score = -infinity;
    for (int index : moveOrder) {
        if (board[index] != '.') continue;
        board[index] = 'O';
        const int score = search(board, 'X', -infinity, infinity, result,
                                 method == Method::AlphaBeta ? &table : nullptr);
        board[index] = '.';
        if (score > result.score) {
            result.move = index;
            result.score = score;
        }
    }
    return result;
}
}
