#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <iterator>


std::string getCurrentTimeStamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void writeToLog(const std::string &message) {
    std::ofstream logFile("/fibudata/app_log.txt", std::ios_base::app);  // appending to the log file
    if (logFile.is_open()) {
        logFile << getCurrentTimeStamp() << " - " << message << std::endl;
        logFile.close();
    }
}

int main() {
    writeToLog("addBooking");
    std::cout << "Access-Control-Allow-Origin: *" << std::endl;  // Allows any origin to access
    std::cout << "Access-Control-Allow-Methods: POST, GET, OPTIONS" << std::endl; // Allowed methods
    std::cout << "Access-Control-Allow-Headers: Content-Type" << std::endl;  // Allowed headers

    // Do not skip whitespace, more configuration may also be needed.
    std::cin >> std::noskipws;

    // Copy all data from cin, using iterators.
    std::istream_iterator<char> begin(std::cin);
    std::istream_iterator<char> end;
    std::string inputData(begin, end);
    // std::getline(std::cin, inputData);

    const char* contentType = getenv("CONTENT_TYPE");
    if (!contentType || std::string(contentType) != "application/json") {
        writeToLog("Invalid content type. Expected application/json.");
        std::string errorResponse = "{\"status\": \"error\", \"message\": \"Invalid content type. Expected application/json.\"}";
        std::cout << "Content-Length: " << errorResponse.length() << std::endl;
        std::cout << "Content-type: application/json" << std::endl << std::endl;
        std::cout << errorResponse;
        std::cout.flush();
        return 0;
    }

    try {
        Json::Value receivedData;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream iss(inputData);
        if (!Json::parseFromStream(builder, iss, &receivedData, &errors)) {
            throw std::runtime_error(errors);
        }

        // Add additional fields
        receivedData["TimeStamp"] = getCurrentTimeStamp();
        receivedData["User"] = "exampleUser"; // Change this as per your requirements

        // Append the data to bookings.json
        std::ofstream file("/fibudata/bookings.json", std::ios_base::app);
        if (file.is_open()) {
            file << receivedData.toStyledString() << std::endl;
            file.close();
            std::string responseBody = "{\"status\": \"success\", \"message\": \"Booking added successfully!\"}";
            std::cout << "Content-Length: " << responseBody.size() << "\r\n";
            std::cout << "Content-type: application/json; charset=utf-8\r\n";
            std::cout << "Access-Control-Allow-Origin: *\r\n";  // Allows any origin to access
            std::cout << "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"; // Allowed methods
            std::cout << "Access-Control-Allow-Headers: Content-Type\r\n";  // Allowed headers
            std::cout << "Connection: close\r\n\r\n";
            std::cout << responseBody;
            writeToLog("addBooking: after responseBody");
        } else {
            std::cout << "{\"status\": \"error\", \"message\": \"Failed to open the bookings file!\"}" << std::endl;
        }
    } catch (const std::exception& e) {
        std::string errorResponse = "{\"status\": \"error\", \"message\": \"" + std::string(e.what()) + "\"}";
        std::cout << "Content-Length: " << errorResponse.length() << std::endl;
        std::cout << "Content-type: application/json" << std::endl << std::endl;
        std::cout << errorResponse;
    }
    // After sending the JSON response
    std::cout.flush();
    writeToLog("before return");
    return 0;
}

