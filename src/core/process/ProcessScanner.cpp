#include "ProcessScanner.h"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

std::vector<GameProcess> ProcessScanner::scan() {
    std::vector<GameProcess> result;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return result;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!Process32FirstW(snapshot, &entry)) {
        CloseHandle(snapshot);
        return result;
    }

    do {
        if (entry.th32ProcessID == 0 || entry.th32ProcessID == 4)
            continue;

        HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);

        std::string exePath;
        bool is64 = true;

        if (proc) {
            wchar_t pathBuf[MAX_PATH]{};
            DWORD size = MAX_PATH;
            if (QueryFullProcessImageNameW(proc, 0, pathBuf, &size)) {
                int len = WideCharToMultiByte(CP_UTF8, 0, pathBuf, -1, nullptr, 0, nullptr, nullptr);
                exePath.resize(len - 1);
                WideCharToMultiByte(CP_UTF8, 0, pathBuf, -1, exePath.data(), len, nullptr, nullptr);
            }
            is64 = isProcess64Bit(proc);
            CloseHandle(proc);
        }

        int nameLen = WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, nullptr, 0, nullptr, nullptr);
        std::string name(nameLen - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, name.data(), nameLen, nullptr, nullptr);

        result.push_back({entry.th32ProcessID, name, exePath, is64});

    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);
    return result;
}

bool ProcessScanner::isProcess64Bit(void* processHandle) {
    BOOL wow64 = FALSE;
    IsWow64Process(static_cast<HANDLE>(processHandle), &wow64);
    // wow64 == TRUE means 32-bit process running under 64-bit Windows
    return !wow64;
}

#else

std::vector<GameProcess> ProcessScanner::scan() {
    // Linux: stub for future ptrace-based implementation
    return {};
}

#endif
