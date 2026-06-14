#include "logger.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
std::string logFilePath = "logs/agent.log";
}

void Logger::configureFile(const std::string& filepath) {
    if (!filepath.empty()) {
        logFilePath = filepath;
    }
}

void Logger::write(const std::string& level, const std::string& message, bool stderrOutput) {
    const std::string line = "[" + level + "] " + message;
    if (stderrOutput) {
        std::cerr << line << std::endl;
    } else {
        std::cout << line << std::endl;
    }

    try {
        const std::filesystem::path path(logFilePath);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        std::ofstream file(logFilePath, std::ios::app);
        if (file.is_open()) {
            file << line << '\n';
        }
    } catch (...) {
        // Logging must never crash the agent in a diagnostics context.
    }
}

void Logger::info(const std::string& message) {
    write("INFO", message);
}

void Logger::warn(const std::string& message) {
    write("WARNING", message);
}

void Logger::error(const std::string& message) {
    write("ERROR", message, true);
}
