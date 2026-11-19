#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "browser.hpp"
#include "cli.hpp"
#include <string>

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc == 2 && std::string(argv[1]) == "--html") return openGame();
    return runMenu();
}
