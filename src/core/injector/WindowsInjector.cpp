#include "WindowsInjector.h"
#include <windows.h>
#include <tlhelp32.h>
#include <filesystem>

bool WindowsInjector::inject(unsigned long pid, const std::string& dllPath) {
    if (!std::filesystem::exists(dllPath)) {
        setError("DLL not found: " + dllPath);
        return false;
    }

    std::filesystem::path absPath = std::filesystem::absolute(dllPath);
    std::string absStr = absPath.string();

    HANDLE proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
                              PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    if (!proc) {
        setError("OpenProcess failed: " + getWinError());
        return false;
    }

    SIZE_T pathSize = absStr.size() + 1;
    void* remoteMem = VirtualAllocEx(proc, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem) {
        CloseHandle(proc);
        setError("VirtualAllocEx failed: " + getWinError());
        return false;
    }

    if (!WriteProcessMemory(proc, remoteMem, absStr.c_str(), pathSize, nullptr)) {
        VirtualFreeEx(proc, remoteMem, 0, MEM_RELEASE);
        CloseHandle(proc);
        setError("WriteProcessMemory failed: " + getWinError());
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE loadLib =
        reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(kernel32, "LoadLibraryA"));

    HANDLE thread = CreateRemoteThread(proc, nullptr, 0, loadLib, remoteMem, 0, nullptr);
    if (!thread) {
        VirtualFreeEx(proc, remoteMem, 0, MEM_RELEASE);
        CloseHandle(proc);
        setError("CreateRemoteThread failed: " + getWinError());
        return false;
    }

    WaitForSingleObject(thread, 5000);

    DWORD exitCode = 0;
    GetExitCodeThread(thread, &exitCode);

    CloseHandle(thread);
    VirtualFreeEx(proc, remoteMem, 0, MEM_RELEASE);
    CloseHandle(proc);

    if (exitCode == 0) {
        setError("LoadLibrary returned NULL — DLL may be invalid or missing dependencies");
        return false;
    }

    m_lastError.clear();
    return true;
}

bool WindowsInjector::eject(unsigned long pid, const std::string& dllName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        setError("CreateToolhelp32Snapshot failed: " + getWinError());
        return false;
    }

    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    HMODULE targetMod = nullptr;

    if (Module32FirstW(snapshot, &me)) {
        do {
            int len = WideCharToMultiByte(CP_UTF8, 0, me.szModule, -1, nullptr, 0, nullptr, nullptr);
            std::string modName(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, me.szModule, -1, modName.data(), len, nullptr, nullptr);

            if (modName == dllName) {
                targetMod = me.hModule;
                break;
            }
        } while (Module32NextW(snapshot, &me));
    }
    CloseHandle(snapshot);

    if (!targetMod) {
        setError("Module not found in target process: " + dllName);
        return false;
    }

    HANDLE proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION, FALSE, pid);
    if (!proc) {
        setError("OpenProcess failed: " + getWinError());
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE freeLib =
        reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(kernel32, "FreeLibrary"));

    HANDLE thread = CreateRemoteThread(proc, nullptr, 0, freeLib, targetMod, 0, nullptr);
    if (!thread) {
        CloseHandle(proc);
        setError("CreateRemoteThread failed: " + getWinError());
        return false;
    }

    WaitForSingleObject(thread, 5000);
    CloseHandle(thread);
    CloseHandle(proc);

    m_lastError.clear();
    return true;
}

std::string WindowsInjector::lastError() const {
    return m_lastError;
}

void WindowsInjector::setError(const std::string& msg) {
    m_lastError = msg;
}

std::string WindowsInjector::getWinError() {
    DWORD code = GetLastError();
    if (code == 0) return "unknown error";

    char* buf = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   reinterpret_cast<char*>(&buf), 0, nullptr);

    std::string msg = buf ? buf : "unknown error";
    LocalFree(buf);
    if (!msg.empty() && msg.back() == '\n') msg.pop_back();
    if (!msg.empty() && msg.back() == '\r') msg.pop_back();
    return msg;
}
