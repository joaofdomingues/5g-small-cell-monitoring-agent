#pragma once

#include <string>

class Logger {
public:
    static void configureFile(const std::string& filepath);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static void write(const std::string& level, const std::string& message, bool stderrOutput = false);
};
