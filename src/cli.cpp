#include "cli.hpp"
#include "benchmark.hpp"
#include "browser.hpp"
#include <fstream>
#include <iostream>

using namespace std;

namespace {
void printMenu() {
    cout << "\nКрестики-нолики\n"
         << "  1. Открыть игру в браузере\n"
         << "  2. Найти ход ноликами\n"
         << "  3. Сравнить алгоритмы на одной позиции\n"
         << "  4. Сравнить алгоритмы на готовых позициях\n"
         << "  5. Сравнить время заполнения AVL и массива\n"
         << "  0. Выйти\n";
}

void printBoard(string board) {
    for (int row = 0; row < 3; ++row) {
        cout << ' ' << board[row * 3] << ' ' << board[row * 3 + 1] << ' ' << board[row * 3 + 2] << '\n';
    }
}

void printResult(Method method, Measurement measurement) {
    auto result = measurement.result;
    cout << methodName(method) << ": строка " << result.move / 3 + 1
         << ", столбец " << result.move % 3 + 1 << ", оценка " << result.score
         << ", узлов " << result.nodes << ", отсечений " << result.cuts
         << ", оценок из памяти " << result.hits << ", " << measurement.microseconds << " мкс\n";
}

void comparePosition(string board, ostream& csv) {
    cout << "\nПозиция " << board << '\n';
    printBoard(board);
    for (auto method : methods) {
        auto measurement = measure(board, method, 100);
        printResult(method, measurement);
        writeSearchRow(csv, board, method, measurement);
    }
}

void finishCsv(ofstream& output, string name) {
    output.close();
    if (output) cout << "\nТаблица сохранена в " << name << ".\n";
    else cerr << "\nНе удалось сохранить " << name << ".\n";
}

void compareExamples() {
    ofstream csv("search_examples.csv");
    writeSearchHeader(csv);
    for (auto board : {"X........", "X...O...X", "XX..O....", "OO.XX.X..", "XOX.OXOX."}) {
        comparePosition(board, csv);
    }
    finishCsv(csv, "search_examples.csv");
}
}

int runMenu() {
    string command;
    while (true) {
        printMenu();
        cout << "Выберите пункт: ";
        if (!getline(cin, command) || command == "0") return 0;
        if (command == "1") {
            openGame();
            continue;
        }
        if (command == "4") {
            compareExamples();
            continue;
        }
        if (command == "5") {
            ofstream csv("build_comparison.csv");
            writeBuildComparison(csv);
            finishCsv(csv, "build_comparison.csv");
            continue;
        }
        if (command != "2" && command != "3") {
            cout << "Такого пункта нет.\n";
            continue;
        }

        cout << "Введите поле по строкам, например XX..O....: ";
        string board;
        if (!getline(cin, board)) return 0;
        string error = positionError(board);
        if (!error.empty()) {
            cout << error << '\n';
            continue;
        }
        if (command == "3") {
            ofstream csv("benchmark.csv");
            writeSearchHeader(csv);
            comparePosition(board, csv);
            finishCsv(csv, "benchmark.csv");
            continue;
        }

        for (int i = 0; i < 4; ++i) cout << i + 1 << ". " << methodName(methods[i]) << '\n';
        cout << "Выберите алгоритм: ";
        if (!getline(cin, command)) return 0;
        if (command.size() != 1 || command[0] < '1' || command[0] > '4') {
            cout << "Такого алгоритма нет.\n";
            continue;
        }

        auto method = methods[command[0] - '1'];
        auto measurement = measure(board, method, 1);
        printResult(method, measurement);
        board[measurement.result.move] = 'O';
        printBoard(board);
    }
}
