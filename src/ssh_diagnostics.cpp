#include "ssh_diagnostics.h"

SSHDiagnostics::SSHDiagnostics(bool enabled) : enabled_(enabled) {}

bool SSHDiagnostics::isEnabled() const {
    return enabled_;
}

std::string SSHDiagnostics::buildCommand(const std::string& host, const std::string& command) const {
    return "ssh " + host + " '" + command + "'";
}
