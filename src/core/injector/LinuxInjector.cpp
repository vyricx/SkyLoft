#include "LinuxInjector.h"

// Linux shared library injection via ptrace + dlopen
// TODO: implement ptrace-based injection for .so files

bool LinuxInjector::inject(unsigned long /*pid*/, const std::string& /*soPath*/) {
    m_lastError = "Linux injection not yet implemented";
    return false;
}

bool LinuxInjector::eject(unsigned long /*pid*/, const std::string& /*soName*/) {
    m_lastError = "Linux ejection not yet implemented";
    return false;
}

std::string LinuxInjector::lastError() const {
    return m_lastError;
}
