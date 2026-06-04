#pragma once
#include <string>

struct GameProcess {
    unsigned long pid;
    std::string name;
    std::string exePath;
    bool is64Bit;
};
