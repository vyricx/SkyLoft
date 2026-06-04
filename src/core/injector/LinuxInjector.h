#pragma once
#include "IInjector.h"

class LinuxInjector : public IInjector {
public:
    bool inject(unsigned long pid, const std::string& dllPath) override;
    bool eject(unsigned long pid, const std::string& dllName) override;
    std::string lastError() const override;

private:
    std::string m_lastError;
};
