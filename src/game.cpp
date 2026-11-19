#include "game.hpp"
#include "sorted_scores.hpp"
#include "tree.hpp"
#include <algorithm>

using namespace std;

namespace {
constexpr int winLines[][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};
constexpr int moveOrder[] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
constexpr int limit = 20;

struct SavedPositions {
    Method method;
    Tree<SavedScore> tree;
    SortedScores sorted;

    bool enabled() {
        return method == Method::Avl || method == Method::Sorted;
    }

    const SavedScore* find(int position) {
        if (method == Method::Avl) return tree.findValue({position, 0});
        return sorted.findValue({position, 0});
    }

    void insert(int position, int score) {
        if (method == Method::Avl) tree.insert({position, score});
        else sorted.insert({position, score});
    }
};

int search(string& board, char turn, int alpha, int beta,
           SearchResult& result, SavedPositions& saved) {
    ++result.nodes;
    int emptyCells = count(board.begin(), board.end(), '.');
    char won = winner(board);
    if (won == 'O') return 1 + emptyCells;
    if (won == 'X') return -1 - emptyCells;
    if (emptyCells == 0) return 0;

    int key = 0;
    if (saved.enabled()) {
        key = 2 * positionCode(board) + (turn == 'O');
        if (auto entry = saved.find(key)) {
            ++result.hits;
            return entry->score;
        }
    }

    int startAlpha = alpha;
    int startBeta = beta;
    bool maximizing = turn == 'O';
    int best = maximizing ? -limit : limit;
    for (int cell : moveOrder) {
        if (board[cell] != '.') continue;

        board[cell] = turn;
        int score = search(board, maximizing ? 'X' : 'O', alpha, beta, result, saved);
        board[cell] = '.';

        if (maximizing) {
            best = max(best, score);
            alpha = max(alpha, best);
        } else {
            best = min(best, score);
            beta = min(beta, best);
        }
        if (saved.method != Method::Minimax && alpha >= beta) {
            ++result.cuts;
            break;
        }
    }

    // Оценка внутри исходных границ точна. Её можно использовать повторно.
    if (saved.enabled() && best > startAlpha && best < startBeta) saved.insert(key, best);
    return best;
}
}

string methodName(Method method) {
    switch (method) {
    case Method::Minimax: return "Минимакс";
    case Method::AlphaBeta: return "Альфа-бета";
    case Method::Avl: return "Альфа-бета + AVL";
    case Method::Sorted: return "Альфа-бета + массив";
    }
    return {};
}

string methodId(Method method) {
    switch (method) {
    case Method::Minimax: return "minimax";
    case Method::AlphaBeta: return "alpha_beta";
    case Method::Avl: return "alpha_beta_avl";
    case Method::Sorted: return "alpha_beta_sorted";
    }
    return {};
}

char winner(string board) {
    for (auto& line : winLines) {
        char first = board[line[0]];
        if (first != '.' && first == board[line[1]] && first == board[line[2]]) return first;
    }
    return '.';
}

int positionCode(string board) {
    int code = 0;
    for (char cell : board) {
        code *= 3;
        if (cell == 'X') code += 1;
        if (cell == 'O') code += 2;
    }
    return code;
}

string positionFromCode(int code) {
    string board(9, '.');
    for (int cell = 8; cell >= 0; --cell) {
        board[cell] = ".XO"[code % 3];
        code /= 3;
    }
    return board;
}

string positionError(string board) {
    if (board.size() != 9 || board.find_first_not_of(".XO") != string::npos) {
        return "Введите девять знаков: X, O или точку.";
    }
    if (winner(board) != '.' || board.find('.') == string::npos) return "Партия уже завершена.";
    int xCount = count(board.begin(), board.end(), 'X');
    int oCount = count(board.begin(), board.end(), 'O');
    if (xCount != oCount + 1) return "Для хода O крестиков должно быть на один больше.";
    return {};
}

SearchResult findMove(string board, Method method) {
    SearchResult result;
    if (!positionError(board).empty()) return result;

    SavedPositions saved{method, {}, {}};
    result.score = -limit;
    for (int cell : moveOrder) {
        if (board[cell] != '.') continue;

        board[cell] = 'O';
        int score = search(board, 'X', -limit, limit, result, saved);
        board[cell] = '.';

        if (score > result.score) {
            result.move = cell;
            result.score = score;
        }
    }
    return result;
}
