#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class Logger {
public:
    Logger(const std::string& filename) : logFile(filename, std::ios_base::app) {
        if (!logFile.is_open()) {
            throw std::runtime_error("Unable to open log file.");
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    template <typename T>
    Logger& operator<<(const T& message) {
        if (logFile.is_open()) {
            logFile << message;
        }
        return *this;
    }

    // Handle std::endl and other manipulators by creating a special overload for function pointers
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (logFile.is_open()) {
            manip(logFile);
        }
        return *this;
    }

private:
    std::ofstream logFile;
};

std::string getCurrentTimeStamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

