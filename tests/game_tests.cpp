#include "game.hpp"
#include "sorted_scores.hpp"
#include "tree.hpp"
#include <cstdlib>
#include <iostream>

using namespace std;

void check(bool condition, string message) {
    if (!condition) {
        cerr << message << '\n';
        exit(1);
    }
}

int main() {
    Tree<int> tree;
    tree.insert(30);
    tree.insert(10);
    tree.insert(20);
    check(tree.height() == 2, "Дерево должно сбалансироваться");
    check(tree.findValue(20) && *tree.findValue(20) == 20, "Поиск значения после поворота");
    check(!tree.findValue(40), "Отсутствующее значение");

    SortedScores sorted;
    check(!sorted.findValue({20, 0}), "Пустой массив");
    for (int key : {30, 10, 20}) check(sorted.insert({key, key / 10}), "Вставка в массив");
    check(!sorted.insert({20, 9}), "Повторный ключ");
    for (int key : {10, 20, 30}) {
        auto entry = sorted.findValue({key, 0});
        check(entry && entry->score == key / 10, "Бинарный поиск оценки");
    }
    for (int key : {0, 15, 40}) check(!sorted.findValue({key, 0}), "Ключ отсутствует в массиве");

    check(winner("X...X...X") == 'X', "Победа по диагонали");
    check(winner(".O..O..O.") == 'O', "Победа по столбцу");
    check(winner("XXX.O...O") == 'X', "Победа по строке");
    for (auto board : {"X........X", "?........", ".........", "XXXOO...."}) {
        check(!positionError(board).empty(), board);
        check(findMove(board, Method::Minimax).move == -1, board);
    }
    check(findMove("OO.XX.X..", Method::AlphaBeta).move == 2, "Выигрышный ход");
    check(findMove("XX..O....", Method::Minimax).move == 2, "Защитный ход");
    check(findMove("XOX.OXOX.", Method::AlphaBeta).score == 0, "Ничья");

    int checked = 0;
    bool usedCache = false;
    for (int code = 0; code < positionCount; ++code) {
        string board = positionFromCode(code);
        check(positionCode(board) == code, "Обратимость кода позиции");
        if (!positionError(board).empty()) continue;

        auto full = findMove(board, Method::Minimax);
        auto pruned = findMove(board, Method::AlphaBeta);
        auto avl = findMove(board, Method::Avl);
        auto array = findMove(board, Method::Sorted);
        check(full.move >= 0 && full.move < 9 && board[full.move] == '.', board);
        for (auto result : {pruned, avl, array}) {
            check(full.move == result.move && full.score == result.score, board);
            check(result.nodes <= full.nodes, board);
        }
        check(pruned.hits == 0, "Поиск без хранения оценок");
        check(avl.nodes == array.nodes && avl.cuts == array.cuts && avl.hits == array.hits, board);
        usedCache = usedCache || avl.hits > 0;
        ++checked;
    }
    check(checked == 2097, "Число допустимых позиций");
    check(usedCache, "Поиск должен использовать сохранённые оценки");
    cout << "Проверено позиций: " << checked << '\n';
}
