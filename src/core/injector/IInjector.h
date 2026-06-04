#pragma once
#include <string>

class IInjector {
public:
    virtual ~IInjector() = default;

    virtual bool inject(unsigned long pid, const std::string& dllPath) = 0;
    virtual bool eject(unsigned long pid, const std::string& dllName) = 0;
    virtual std::string lastError() const = 0;
};
