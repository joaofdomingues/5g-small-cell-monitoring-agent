#pragma once

#include <string>

class SSHDiagnostics {
public:
    explicit SSHDiagnostics(bool enabled = true);
    bool isEnabled() const;
    std::string buildCommand(const std::string& host, const std::string& command) const;

private:
    bool enabled_;
};
