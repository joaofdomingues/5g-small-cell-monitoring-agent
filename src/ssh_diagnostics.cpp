#include "ssh_diagnostics.h"
#include "logger.h"

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

// Execute a shell command and capture stdout (up to ~4 KB).
// Returns {command, output, success}.
SSHResult execCommand(const std::string& label, const std::string& cmd) {
    SSHResult result;
    result.command = label;

    std::array<char, 256> buffer{};
    std::string output;

#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");  // NOLINT(cert-env33-c)
#endif

    if (!pipe) {
        result.output  = "popen failed";
        result.success = false;
        return result;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        output += buffer.data();
        if (output.size() > 4096) break; // Guard against runaway output.
    }

#ifdef _WIN32
    const int rc = _pclose(pipe);
#else
    const int rc = pclose(pipe);
#endif

    // Trim trailing newline for cleaner log output.
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
        output.pop_back();
    }

    result.output  = output.empty() ? "(no output)" : output;
    result.success = (rc == 0);
    return result;
}

// Diagnostic commands run on the target node (locally or via SSH).
const std::vector<std::pair<std::string, std::string>> DIAG_COMMANDS = {
    {"hostname",  "hostname"},
    {"uptime",    "uptime"},
    {"disk",      "df -h / | tail -1"},
    {"network",   "ip addr show | head -20"},
};

} // namespace

// ---------------------------------------------------------------------------
// SSHDiagnostics
// ---------------------------------------------------------------------------

SSHDiagnostics::SSHDiagnostics(bool enabled) : enabled_(enabled) {}

bool SSHDiagnostics::isEnabled() const { return enabled_; }

std::string SSHDiagnostics::buildCommand(const std::string& host,
                                         const std::string& command) const {
    return "ssh " + host + " '" + command + "'";
}

SSHResult SSHDiagnostics::runLocalCommand(const std::string& command) const {
    return execCommand(command, command);
}

SSHResult SSHDiagnostics::runRemoteCommand(const std::string& host,
                                            const std::string& user,
                                            const std::string& command) const {
    // Build a non-interactive SSH invocation with a short connect timeout.
    // StrictHostKeyChecking=no is intentional for a lab/portfolio environment;
    // a production deployment would use known_hosts or a CA certificate.
    const std::string sshCmd =
        "ssh -o ConnectTimeout=5 "
        "-o StrictHostKeyChecking=no "
        "-o BatchMode=yes "
        + user + "@" + host + " '" + command + "' 2>&1";
    return execCommand(command, sshCmd);
}

std::vector<SSHResult> SSHDiagnostics::runDiagnostics(const std::string& host,
                                                       const std::string& user) const {
    std::vector<SSHResult> results;

    if (!enabled_) {
        Logger::warn("SSH diagnostics are disabled — skipping remote checks.");
        return results;
    }

    const bool isLocal = (host == "localhost" || host == "127.0.0.1");

    if (isLocal) {
        Logger::info("SSH diagnostics: running in local mode (host=" + host + ")");
    } else {
        Logger::info("SSH diagnostics: connecting to " + user + "@" + host);
    }

    for (const auto& [label, cmd] : DIAG_COMMANDS) {
        SSHResult r = isLocal ? runLocalCommand(cmd)
                              : runRemoteCommand(host, user, cmd);
        Logger::info("  [" + label + "] " + r.output);
        results.push_back(std::move(r));
    }

    return results;
}
