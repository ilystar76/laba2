#pragma once
#include <string>

enum class Method { Minimax, AlphaBeta, Avl, Sorted };
constexpr Method methods[] = {Method::Minimax, Method::AlphaBeta, Method::Avl, Method::Sorted};
constexpr int positionCount = 19683;

std::string methodName(Method method);
std::string methodId(Method method);

struct SearchResult {
    int move = -1;
    int score = 0;
    int nodes = 0;
    int cuts = 0;
    int hits = 0;
};

char winner(std::string board);
int positionCode(std::string board);
std::string positionFromCode(int code);
std::string positionError(std::string board);
SearchResult findMove(std::string board, Method method);
