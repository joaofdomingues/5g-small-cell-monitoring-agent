#pragma once

#include <string>
#include <vector>

struct SSHResult {
    std::string command;
    std::string output;
    bool        success{false};
};

class SSHDiagnostics {
public:
    explicit SSHDiagnostics(bool enabled = true);

    bool isEnabled() const;

    // Build an SSH command string (used for logging / dry-run display).
    std::string buildCommand(const std::string& host,
                             const std::string& command) const;

    // Run a set of diagnostic commands on a remote host via the system SSH
    // binary.  In dry-run mode (host == "localhost" / "127.0.0.1") the
    // commands are executed locally so the project works without real
    // infrastructure.
    std::vector<SSHResult> runDiagnostics(const std::string& host,
                                          const std::string& user) const;

private:
    bool enabled_;

    SSHResult runLocalCommand(const std::string& command) const;
    SSHResult runRemoteCommand(const std::string& host,
                               const std::string& user,
                               const std::string& command) const;
};
