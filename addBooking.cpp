#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <cstdlib>


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

#include <iostream>
#include <fstream>

int appendToJsonArrayFile(const std::string& filename, const std::string& new_json_data) {
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::ate);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open the file!" << std::endl;
        return 0;
    }

    // Check if the file is empty
    if (file.tellg() == 0) {
        // If the file is empty, initialize it with the new JSON data enclosed in square brackets
        file << "[\n" << new_json_data << "\n]";
    } else {
        // If the file is not empty, move two characters backward (to overwrite the closing bracket)
        file.seekp(-2, std::ios::end);
        
        // Append a comma, the new JSON data, and then the closing bracket
        file << ",\n" << new_json_data << "\n]";
    }

    file.close();
    return 1;
}


/*
int appendToJsonArrayFile(const std::string& filename, const std::string& new_json_data) {
    std::fstream file(filename, std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file!" << std::endl;
        return  0;
    }

    // If the file is empty, initialize it as an array
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        file << "[\n]";
    }

    // Seek to the end minus 2 positions
    file.seekp(-2, std::ios::end);
    char prevChar;
    file.seekg(-2, std::ios::cur);

    file.get(prevChar);
    if (prevChar != '[') {
        file << ",";
    }
    file << new_json_data << "\n]";

    // Append the new JSON data
    file << "," << new_json_data << "\n]";

    file.close();
    return 1;
}
*/

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
        const char* username = std::getenv("REMOTE_USER");
        if (!username) {
            username = std::getenv("HTTP_REMOTE_USER");
        }   
        if (username) {
            writeToLog (username);
            receivedData["User"] = username; // Change this as per your requirements
        } else {
            receivedData["User"] = "";
        }

        std::string client = receivedData["Mandant"].asString();
        // Append the data to bookings.json
        std::ofstream file("/fibudata/"+client+"-bookings.json", std::ios_base::app);
        if(appendToJsonArrayFile("/fibudata/"+client+"-bookings.json", receivedData.toStyledString())){
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

