#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#include "browser.hpp"
#include "game.hpp"
#include <set>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

void writeBoard(ostream& output, string board, set<int>& written) {
    int code = positionCode(board);
    if (!written.insert(code).second) return;

    char won = winner(board);
    bool finished = won != '.' || board.find('.') == string::npos;
    string nextBoards[9];
    output << "<section id=\"p" << code << "\"><h2>";
    if (won != '.') output << "Победа " << won;
    else if (finished) output << "Ничья";
    else output << "Ваш ход за X";
    output << "</h2><div class=\"board\">";

    for (int cell = 0; cell < 9; ++cell) {
        if (board[cell] != '.' || finished) {
            output << "<span>" << (board[cell] == '.' ? ' ' : board[cell]) << "</span>";
            continue;
        }

        string next = board;
        next[cell] = 'X';
        if (winner(next) == '.' && next.find('.') != string::npos) {
            int move = findMove(next, Method::Avl).move;
            next[move] = 'O';
        }
        nextBoards[cell] = next;
        output << "<a href=\"#p" << positionCode(next) << "\" aria-label=\"Строка "
               << cell / 3 + 1 << ", столбец " << cell % 3 + 1 << "\">·</a>";
    }
    output << "</div><a class=\"reset\" href=\"#p0\">Новая игра</a></section>\n";

    for (auto& next : nextBoards) {
        if (!next.empty()) writeBoard(output, next, written);
    }
}

bool makePage(filesystem::path path) {
    ofstream output(path);
    if (!output) return false;

    output << R"html(<!doctype html>
<html lang="ru">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Крестики-нолики</title>
<style>
body { background: #151625; color: #f5f3ff; font: 1rem system-ui; text-align: center; margin: 2rem 1rem; }
main { max-width: 24rem; margin: auto; }
section { display: none; padding: 1rem 1.25rem 1.5rem; background: #24263a; border-radius: 1rem; }
section:target { display: block; }
.start { display: block; }
section:target ~ .start { display: none; }
.board { display: grid; grid-template-columns: repeat(3, 1fr); gap: .5rem; margin-bottom: 1.25rem; }
.board > * { aspect-ratio: 1; display: grid; place-items: center; border-radius: .6rem; background: #333650; font-size: 2.4rem; text-decoration: none; }
a { color: #d6c4ff; }
.board a:hover, .board a:focus-visible { background: #4b3d70; }
</style>
<main><h1>Крестики-нолики</h1>
)html";
    set<int> written;
    writeBoard(output, ".........", written);
    output << "<section class=\"start\"><h2>Игра против компьютера</h2>"
           << "<a href=\"#p0\">Начать игру</a></section></main>\n</html>\n";
    output.close();
    return bool(output);
}
int openGame() {
    auto path = filesystem::absolute("laba2.html");
    cout << "Подготавливаю поле для браузера…\n";
    if (!makePage(path)) {
        cerr << "Не удалось записать laba2.html. Запустите программу из папки с правом записи.\n";
        return 1;
    }

    cout << "Откройте файл: " << path.u8string() << '\n';
#ifdef _WIN32
    auto status = ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(status) <= 32) cout << "Откройте файл вручную.\n";
#endif
    return 0;
}
