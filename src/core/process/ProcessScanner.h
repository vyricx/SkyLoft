#pragma once
#include <vector>
#include "GameProcess.h"

class ProcessScanner {
public:
    static std::vector<GameProcess> scan();

private:
#ifdef _WIN32
    static bool isProcess64Bit(void* processHandle);
#endif
};
